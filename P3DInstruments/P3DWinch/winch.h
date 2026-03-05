
#pragma once

#include <cmath>
#include <string>
#include <memory>
#include <algorithm>
#include "state_vector.h"
#include "v3d.h"
#include "quaternion.h"
#include "Spline.h"


// Skylaunch winch launch simulation for glider.
//
// Models a Skylaunch-type ground-based winch drivetrain:
//   Engine -> Torque Converter -> TH400 3-speed Auto -> Final Drive -> Drum -> Cable
//
// Engine:  GM 7.4L (454ci) Big Block V8, LPG fuelled
// Transmission:  Turbo-Hydramatic 400 (2.48 / 1.48 / 1.00)
//
// The glider controls cable speed - the winch reacts to the load.
// For a given cable speed and throttle position the model solves for the
// engine equilibrium RPM, then propagates torque through the drivetrain
// to give cable tension.
//
// Features:
// - Full drivetrain model with torque converter fluid coupling
// - Automatic gear selection (highest gear with RPM >= 1400)
// - Variable drum radius from spiral cable winding
// - Default throttle profile with steep-angle reduction
// - External throttle API for scripting
// - Automatic back-release if cable pulls backwards relative to glider
// - Weak link simulation

class Winch {

    // --- Drivetrain spline data (static, shared across all instances) ---
    static float s_engine_rpm[10];
    static float s_engine_nm[10];
    static float s_tc_sr[13];
    static float s_tc_tr[13];
    static float s_tc_kf[13];

    // --- Drivetrain constants ---
    static constexpr float IDLE_RPM = 700.0f;
    static constexpr float MAX_RPM = 5000.0f;
    static constexpr float IDLE_TORQUE_FRAC = 0.25f;
    static constexpr float THROTTLE_EXPONENT = 1.5f;

    static constexpr float TRANS_RATIO_1 = 2.48f;
    static constexpr float TRANS_RATIO_2 = 1.48f;
    static constexpr float TRANS_RATIO_3 = 1.00f;
    static constexpr float TRANS_EFFICIENCY = 0.95f;

    static constexpr float FINAL_DRIVE_RATIO = 4.0f;
    static constexpr float FINAL_DRIVE_EFFICIENCY = 0.97f;

    static constexpr float DRUM_INNER_RADIUS = 0.35f;   // m
    static constexpr float DRUM_WIDTH = 0.20f;           // m
    static constexpr float CABLE_DIAMETER = 0.0046f;     // m (4.6 mm)
    static constexpr float DRUM_CABLE_LENGTH = 2000.0f;  // m
    static constexpr int   WRAPS_PER_LAYER = 43;         // int(0.20 / 0.0046)

    static constexpr float PI = 3.14159265358979f;

    // --- Per-instance spline objects (mutable state: klo/khi) ---
    std::unique_ptr<Spline<float>> engine_spline;
    std::unique_ptr<Spline<float>> tr_spline;
    std::unique_ptr<Spline<float>> kf_spline;

    // --- Winch state ---
    V3d<float> winch_position;  // Position of winch drum in earth frame (m)
    float weak_link;            // Tension at which weak link breaks (N)
    float cable_length;         // Total cable length on drum (m)
    bool  engaged;              // Is the winch cable currently engaged
    float cable_out;            // Current distance from hook to winch (m)
    float tension;              // Current cable tension (N)
    std::string release_reason; // Why cable was released
    float initial_cable_out;    // Cable distance at engage time (for throttle ramp)

    // --- Throttle state ---
    float throttle;             // Current throttle position (0.0 - 1.0)
    bool  throttle_override;    // Is external throttle override active
    float cable_angle;          // Current cable angle below horizontal (radians)

    // --- Drivetrain result from solver ---
    struct SolveResult {
        bool  valid;
        float cable_tension_n;
        float engine_rpm;
        int   gear;
    };

    // --- Private methods ---
    void  init_splines();
    float engine_torque(float rpm, float thr);
    float tc_torque_ratio(float sr);
    float tc_k_factor(float sr);
    float tc_pump_torque(float pump_rpm, float sr);
    float tc_turbine_torque(float pump_rpm, float sr);
    float drum_effective_radius(float cable_out_m) const;
    float drum_rpm(float cable_speed, float cable_out_m) const;
    float gear_ratio(int gear) const;

    SolveResult solve_gear(int gear, float cable_speed, float thr, float cable_out_m);
    SolveResult solve(float cable_speed, float thr, float cable_out_m);

    float default_throttle() const;

    V3d<float> get_hook_velocity(const StateVector<float>& state, const V3d<float>& hook_body) const;

public:

    Winch(const V3d<float>& winch_position = V3d<float>(1000.0f, 0.0f, 0.0f),
          float weak_link = 10000.0f,
          float cable_length = 1500.0f);

    // Move only (Spline is non-copyable)
    Winch(Winch&& other) noexcept;
    Winch& operator=(Winch&& other) noexcept;
    Winch(const Winch&) = delete;
    Winch& operator=(const Winch&) = delete;

    // Engage the winch cable.
    void engage(float initial_cable_out = -1.0f);

    // Release the cable.
    void release(const std::string& reason = "manual");

    // Calculate winch cable forces and moments.
    void calculate_forces(const StateVector<float>& state, const V3d<float>& hook_position_body,
                         V3d<float>& forces_body, V3d<float>& moments_body);

    // --- External throttle API ---
    void  set_throttle(float t);
    void  clear_throttle_override();
    float get_throttle() const { return throttle; }
    float get_cable_angle() const { return cable_angle; }
    bool  is_engaged() const { return engaged; }
    float get_tension() const { return tension; }
};
