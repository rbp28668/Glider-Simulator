
#include "winch.h"
#include <cmath>
#include <algorithm>
#include <functional>


// ---------------------------------------------------------------------------
//  Static spline data arrays
//  These persist for the lifetime of the program and are shared by all
//  Winch instances.  Spline<float> stores non-owning pointers to them.
// ---------------------------------------------------------------------------

// Engine:  GM 7.4L V8 on LPG - full-throttle torque curve
// Published: 250 kW @ 4600 RPM, 597 Nm @ 3400 RPM
float Winch::s_engine_rpm[10] = { 700, 1000, 1500, 2000, 2500, 3000, 3400, 4000, 4600, 5000 };
float Winch::s_engine_nm[10]  = { 300,  380,  455,  515,  560,  588,  597,  570,  519,  470 };

// Torque converter: TH400-style fluid coupling
// Speed ratio SR = N_turbine / N_pump
float Winch::s_tc_sr[13] = { 0.00f, 0.10f, 0.20f, 0.30f, 0.40f, 0.50f,
                              0.60f, 0.70f, 0.80f, 0.87f, 0.93f, 0.97f, 1.00f };

// Torque ratio TR(SR) - multiplication from ~2.1 at stall to 1.0 at coupling
float Winch::s_tc_tr[13] = { 2.10f, 2.05f, 1.97f, 1.86f, 1.73f, 1.58f,
                              1.41f, 1.24f, 1.10f, 1.00f, 1.00f, 1.00f, 1.00f };

// K-factor K(SR) in RPM/sqrt(Nm)
// T_pump = (N_pump / K)^2
float Winch::s_tc_kf[13] = { 86.8f,  88.0f,  91.0f,  95.0f, 100.0f, 107.0f,
                             117.0f, 132.0f, 155.0f, 195.0f, 240.0f, 310.0f, 420.0f };


// ---------------------------------------------------------------------------
//  File-local Brent's method root finder
// ---------------------------------------------------------------------------

namespace {

template<typename F>
float brent_solve(F f, float a, float b, float tol = 1e-3f, int max_iter = 50) {
    float fa = f(a);
    float fb = f(b);

    if (fa * fb > 0.0f)
        return (a + b) * 0.5f;  // no sign change - return midpoint as fallback

    if (std::abs(fa) < std::abs(fb)) {
        std::swap(a, b);
        std::swap(fa, fb);
    }

    float c = a, fc = fa;
    bool mflag = true;
    float d = 0.0f;

    for (int i = 0; i < max_iter; i++) {
        if (std::abs(b - a) < tol)
            return b;

        float s;
        if (fa != fc && fb != fc) {
            // Inverse quadratic interpolation
            s = a * fb * fc / ((fa - fb) * (fa - fc))
              + b * fa * fc / ((fb - fa) * (fb - fc))
              + c * fa * fb / ((fc - fa) * (fc - fb));
        } else {
            // Secant method
            s = b - fb * (b - a) / (fb - fa);
        }

        // Conditions for bisection fallback
        float mid = (3.0f * a + b) / 4.0f;
        bool between = (mid < b) ? (s >= mid && s <= b) : (s >= b && s <= mid);
        bool cond1 = !between;
        bool cond2 = mflag && std::abs(s - b) >= std::abs(b - c) / 2.0f;
        bool cond3 = !mflag && std::abs(s - b) >= std::abs(c - d) / 2.0f;
        bool cond4 = mflag && std::abs(b - c) < tol;
        bool cond5 = !mflag && std::abs(c - d) < tol;

        if (cond1 || cond2 || cond3 || cond4 || cond5) {
            s = (a + b) / 2.0f;
            mflag = true;
        } else {
            mflag = false;
        }

        float fs = f(s);
        d = c;
        c = b;
        fc = fb;

        if (fa * fs < 0.0f) {
            b = s;
            fb = fs;
        } else {
            a = s;
            fa = fs;
        }

        if (std::abs(fa) < std::abs(fb)) {
            std::swap(a, b);
            std::swap(fa, fb);
        }
    }

    return b;
}

}  // anonymous namespace


// ---------------------------------------------------------------------------
//  Spline initialisation
// ---------------------------------------------------------------------------

void Winch::init_splines() {
    engine_spline = std::make_unique<Spline<float>>(s_engine_rpm, s_engine_nm, 10);
    tr_spline     = std::make_unique<Spline<float>>(s_tc_sr, s_tc_tr, 13);
    kf_spline     = std::make_unique<Spline<float>>(s_tc_sr, s_tc_kf, 13);
}


// ---------------------------------------------------------------------------
//  Constructor
// ---------------------------------------------------------------------------

Winch::Winch(const V3d<float>& winch_position, float weak_link, float cable_length)
    : winch_position(winch_position)
    , weak_link(weak_link)
    , cable_length(cable_length)
    , engaged(false)
    , cable_out(0.0f)
    , tension(0.0f)
    , throttle(0.0f)
    , throttle_override(false)
    , cable_angle(0.0f)
{
    init_splines();
}


// ---------------------------------------------------------------------------
//  Move constructor / assignment
// ---------------------------------------------------------------------------

Winch::Winch(Winch&& other) noexcept
    : engine_spline(std::move(other.engine_spline))
    , tr_spline(std::move(other.tr_spline))
    , kf_spline(std::move(other.kf_spline))
    , winch_position(other.winch_position)
    , weak_link(other.weak_link)
    , cable_length(other.cable_length)
    , engaged(other.engaged)
    , cable_out(other.cable_out)
    , tension(other.tension)
    , release_reason(std::move(other.release_reason))
    , throttle(other.throttle)
    , throttle_override(other.throttle_override)
    , cable_angle(other.cable_angle)
{
}

Winch& Winch::operator=(Winch&& other) noexcept {
    if (this != &other) {
        engine_spline   = std::move(other.engine_spline);
        tr_spline       = std::move(other.tr_spline);
        kf_spline       = std::move(other.kf_spline);
        winch_position  = other.winch_position;
        weak_link       = other.weak_link;
        cable_length    = other.cable_length;
        engaged         = other.engaged;
        cable_out       = other.cable_out;
        tension         = other.tension;
        release_reason  = std::move(other.release_reason);
        throttle        = other.throttle;
        throttle_override = other.throttle_override;
        cable_angle     = other.cable_angle;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  Engage / Release
// ---------------------------------------------------------------------------

void Winch::engage(float initial_cable_out_param) {
    engaged = true;
    release_reason = "";
    if (initial_cable_out_param > 0)
        cable_out = initial_cable_out_param;
}

void Winch::release(const std::string& reason) {
    engaged = false;
    tension = 0.0f;
    release_reason = reason;
}


// ---------------------------------------------------------------------------
//  Throttle API
// ---------------------------------------------------------------------------

void Winch::set_throttle(float t) {
    throttle = std::max(0.0f, std::min(1.0f, t));
}




// ---------------------------------------------------------------------------
//  Engine torque model
// ---------------------------------------------------------------------------

float Winch::engine_torque(float rpm, float thr) {
    // Clamp RPM to spline range (strict < last point for Spline assert)
    rpm = std::max(IDLE_RPM, std::min(rpm, MAX_RPM - 1.0f));

    float wot = engine_spline->point(rpm);

    // Butterfly-valve flow characteristic
    float eff = powf(std::max(0.0f, std::min(1.0f, thr)), THROTTLE_EXPONENT);

    // Idle torque fraction - engine produces some torque even at closed throttle
    float idle_nm = engine_spline->point(IDLE_RPM) * IDLE_TORQUE_FRAC;

    return idle_nm + (wot - idle_nm) * eff;
}


// ---------------------------------------------------------------------------
//  Torque converter
// ---------------------------------------------------------------------------

float Winch::tc_torque_ratio(float sr) {
    sr = std::max(0.0f, std::min(sr, 0.999f));
    return tr_spline->point(sr);
}

float Winch::tc_k_factor(float sr) {
    sr = std::max(0.0f, std::min(sr, 0.999f));
    return kf_spline->point(sr);
}

float Winch::tc_pump_torque(float pump_rpm, float sr) {
    float k = tc_k_factor(sr);
    return (pump_rpm / k) * (pump_rpm / k);   // T_pump = (N_pump / K)^2
}

float Winch::tc_turbine_torque(float pump_rpm, float sr) {
    return tc_pump_torque(pump_rpm, sr) * tc_torque_ratio(sr);
}


// ---------------------------------------------------------------------------
//  Drum model - variable effective radius from spiral winding
// ---------------------------------------------------------------------------

float Winch::drum_effective_radius(float cable_out_m) const {
    float on_drum = DRUM_CABLE_LENGTH - cable_out_m;
    if (on_drum <= 0.0f)
        return DRUM_INNER_RADIUS;

    float remaining = on_drum;
    int layer = 0;
    while (remaining > 0.0f) {
        float r_centre = DRUM_INNER_RADIUS + (layer + 0.5f) * CABLE_DIAMETER;
        float layer_cap = WRAPS_PER_LAYER * 2.0f * PI * r_centre;
        if (remaining <= layer_cap) {
            float frac = remaining / layer_cap;
            return DRUM_INNER_RADIUS + (layer + frac) * CABLE_DIAMETER;
        }
        remaining -= layer_cap;
        layer++;
    }

    return DRUM_INNER_RADIUS + layer * CABLE_DIAMETER;
}

float Winch::drum_rpm(float cable_speed, float cable_out_m) const {
    float r = drum_effective_radius(cable_out_m);
    return cable_speed / r * 30.0f / PI;   // rad/s -> RPM
}

float Winch::gear_ratio(int gear) const {
    switch (gear) {
        case 1: return TRANS_RATIO_1;
        case 2: return TRANS_RATIO_2;
        case 3: return TRANS_RATIO_3;
        default: return 1.0f;
    }
}


// ---------------------------------------------------------------------------
//  Per-gear drivetrain solver
//  Finds engine RPM where T_engine(RPM, throttle) = T_pump(RPM, SR)
// ---------------------------------------------------------------------------

Winch::SolveResult Winch::solve_gear(int gear, NumberT cable_speed, float thr, NumberT cable_out_m) {
    SolveResult result = { false, 0.0f, 0.0f, gear };

    float r_eff = drum_effective_radius(cable_out_m);
    float d_rpm = drum_rpm(cable_speed, cable_out_m);
    float trans_out_rpm = d_rpm * FINAL_DRIVE_RATIO;
    float ratio = gear_ratio(gear);
    float turbine_rpm = trans_out_rpm * ratio;

    // Balance function: engine torque - pump torque = 0 at equilibrium
    auto balance = [&](float eng_rpm) -> float {
        float sr = (eng_rpm > 0.0f) ? (turbine_rpm / eng_rpm) : 0.0f;
        sr = std::max(0.0f, std::min(sr, 0.999f));
        return engine_torque(eng_rpm, thr) - tc_pump_torque(eng_rpm, sr);
    };

    float rpm_lo = std::max(IDLE_RPM, turbine_rpm + 1.0f);
    float rpm_hi = MAX_RPM;
    if (rpm_lo >= rpm_hi)
        return result;

    // Scan for a sign change across the RPM range
    constexpr int N_SCAN = 40;
    float pts[N_SCAN];
    float vals[N_SCAN];
    for (int i = 0; i < N_SCAN; i++) {
        pts[i] = rpm_lo + (rpm_hi - rpm_lo) * i / (N_SCAN - 1);
        vals[i] = balance(pts[i]);
    }

    bool found = false;
    float root = 0.0f;
    for (int i = 0; i < N_SCAN - 1; i++) {
        if (vals[i] * vals[i + 1] < 0.0f) {
            root = brent_solve(balance, pts[i], pts[i + 1]);
            found = true;
            break;
        }
    }
    if (!found)
        return result;

    // Compute drivetrain torques at equilibrium
    float eng_rpm = root;
    float sr = std::max(0.0f, std::min(turbine_rpm / eng_rpm, 0.999f));
    float t_turb = tc_turbine_torque(eng_rpm, sr);
    float t_trans = t_turb * ratio * TRANS_EFFICIENCY;
    float t_drum = t_trans * FINAL_DRIVE_RATIO * FINAL_DRIVE_EFFICIENCY;
    float cable_tension = t_drum / r_eff;

    result.valid = true;
    result.cable_tension_n = cable_tension;
    result.engine_rpm = eng_rpm;
    return result;
}


// ---------------------------------------------------------------------------
//  Auto gear selection - prefer highest gear with RPM >= 1400
// ---------------------------------------------------------------------------

Winch::SolveResult Winch::solve(NumberT cable_speed, float thr, NumberT cable_out_m) {
    SolveResult best = { false, 0.0f, 0.0f, 0 };

    if (cable_speed <= 0.0f)
        return best;

    for (int g = 1; g <= 3; g++) {
        auto r = solve_gear(g, cable_speed, thr, cable_out_m);
        if (!r.valid)
            continue;
        if (r.engine_rpm < IDLE_RPM || r.engine_rpm > MAX_RPM)
            continue;

        if (!best.valid) {
            best = r;
        } else if (r.engine_rpm >= 1400.0f && r.gear > best.gear) {
            best = r;
        }
    }

    return best;
}



// ---------------------------------------------------------------------------
//  Hook velocity in body frame: V_hook = V_cg + omega x r_hook
// ---------------------------------------------------------------------------

V3d<NumberT> Winch::get_hook_velocity(const StateVector<NumberT>& state, const V3d<NumberT>& hook_body) const {
    auto u = state.velocity()[0];
    auto v = state.velocity()[1];
    auto w = state.velocity()[2];
    auto p = state.angular_velocity()[0];
    auto q = state.angular_velocity()[1];
    auto r = state.angular_velocity()[2];

    V3d<NumberT> omega_cross_r(
        q * hook_body[2] - r * hook_body[1],
        r * hook_body[0] - p * hook_body[2],
        p * hook_body[1] - q * hook_body[0]
    );

    return V3d<NumberT>(
        u + omega_cross_r[0],
        v + omega_cross_r[1],
        w + omega_cross_r[2]
    );
}


// ---------------------------------------------------------------------------
//  Calculate winch cable forces and moments
// ---------------------------------------------------------------------------

void Winch::calculate_forces(const StateVector<NumberT>& state, const V3d<NumberT>& hook_position_body,
                             V3d<NumberT>& forces_body, V3d<NumberT>& moments_body) {

    // No force if not engaged
    if (!engaged) return;

    // Get hook position in earth frame
    auto orientation = state.orientation();
    auto hook_earth_offset = orientation.rotate_vector(hook_position_body);
    auto aircraft_pos = state.position();

    auto hook_earth = V3d<NumberT>(
        aircraft_pos[0] + hook_earth_offset[0],
        aircraft_pos[1] + hook_earth_offset[1],
        aircraft_pos[2] + hook_earth_offset[2]
    );

    // Vector from hook to winch (cable direction)
    auto cable_vec = V3d<NumberT>(
        winch_position[0] - hook_earth[0],
        winch_position[1] - hook_earth[1],
        winch_position[2] - hook_earth[2]
    );

    // Cable length (distance from hook to winch)
    auto cable_distance = sqrt(cable_vec[0] * cable_vec[0]
        + cable_vec[1] * cable_vec[1]
        + cable_vec[2] * cable_vec[2]);

    if (cable_distance < 100.0f) {
        release("cable_run_out");
        return;
    }

    // Unit vector along cable (from hook towards winch)
    auto cable_unit = V3d<NumberT>(
        cable_vec[0] / cable_distance,
        cable_vec[1] / cable_distance,
        cable_vec[2] / cable_distance
    );

    // Transform cable direction to body frame
    auto cable_body = orientation.rotate_vector_inverse(cable_unit);

    // Back-release: cable pulling backwards (negative X in body frame)
    if (cable_body[0] < 0) {
        release("back_release");
        return;
    }

    // Update cable out length
    cable_out = cable_distance;

    // Check if cable has run out
    if (cable_out > cable_length) {
        release("cable_run_out");
        return;
    }

    // Calculate cable angle below horizontal (NED: Z positive = down)
    // cable_vec.z > 0 when winch is below hook (glider above ground)
    NumberT horiz_dist = sqrt(cable_vec[0] * cable_vec[0] + cable_vec[1] * cable_vec[1]);
    if (horiz_dist > 0.1f) {
        cable_angle = atan2(cable_vec[2], horiz_dist);
    }

    // Get hook velocity in earth frame
    auto hook_vel_body = get_hook_velocity(state, hook_position_body);
    auto hook_vel_earth = orientation.rotate_vector(hook_vel_body);

    // Velocity component along cable (positive = towards winch)
    auto v_cable = (
        hook_vel_earth[0] * cable_unit[0] +
        hook_vel_earth[1] * cable_unit[1] +
        hook_vel_earth[2] * cable_unit[2]
    );

    // Solve drivetrain for cable tension
    if (v_cable < 0.0f) {
        // Cable slack - glider moving away from winch
        tension = 100.0f;
    } else {
        // Use drivetrain solver (minimum 0.5 m/s for stability near stall)
        NumberT solver_speed = std::max(NumberT(0.5f), v_cable);
        auto result = solve(solver_speed, throttle, cable_out);
        tension = result.valid ? result.cable_tension_n : 100.0f;
    }

    // Check weak link
    if (tension > weak_link) {
        release("weak_link");
        return;
    }

    // Apply force along cable direction (in earth frame, towards winch)
    auto force_earth = V3d<NumberT>(
        tension * cable_unit[0],
        tension * cable_unit[1],
        tension * cable_unit[2]
    );

    // Transform force to body frame
    forces_body = orientation.rotate_vector_inverse(force_earth);

    // Calculate moment about CG
    auto arm = hook_position_body;
    moments_body = V3d<NumberT>(
        arm[1] * forces_body[2] - arm[2] * forces_body[1],
        arm[2] * forces_body[0] - arm[0] * forces_body[2],
        arm[0] * forces_body[1] - arm[1] * forces_body[0]
    );
}
