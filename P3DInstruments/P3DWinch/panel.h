#pragma once

#include <cmath>
#include <algorithm>

#include "aerofoil.h"
#include "v3d.h"
#include "state_vector.h"
#include "aircraft_params.h"
#include "world.h"
#include "control_inputs.h"
#include "local_math.h"

// Represents a lifting surface panel on an aircraft.
// Panel is part of a wing. It may be extended to include control surfaces such as ailerons or airbrakes.
// Subclasses can override hook methods to modify behavior:
// - modify_aoa(): Adjust angle of attack (e.g., for ailerons)
// - modify_coefficients(): Adjust Cl, Cd, Cm (e.g., for spoiler lift reduction)
// - additional_drag(): Add extra drag (e.g., for deployed airbrakes)
class Panel
{

    // Minimum airspeed for aerodynamic calculations (m/s)
    // Below this, forces are scaled to zero to prevent numerical instability
    const float MIN_AIRSPEED = 1.0f;

    // Maximum force magnitude per panel (N) - prevents runaway
    const float MAX_PANEL_FORCE = 50000.0f;

protected:
    float area;               // panel area (m²)
    float mid_span;           // how far outboard the panel is from the aircraft centerline (m)
    float quater_chord;       // distance from datum to quarter chord (m)
    float incidence;          // geometric incidence angle of panel (radians)
    const Aerofoil &rootFoil; // root aerofoil
    const Aerofoil &tipFoil;
    float interp;     // interpolation factor between root and tip aerofoils (0.0 = root, 1.0 = tip)
    float mean_chord; // mean aerodynamic chord (m)

    float clamp(float value, float min_val, float max_val) const
    {
        // Clamp value to range [min_val, max_val].
        return std::max(min_val, std::min(max_val, value));
    }

    float safe_value(float value, float dflt = 0.0f)
    {
        // Return default if value is NaN or Inf.
        return (std::isnan(value) || std::isinf(value)) ? dflt : value;
    }



public:
    Panel(float area, float mid_span, float quater_chord, float mean_chord, float incidenceDegrees, const Aerofoil &rootFoil, const Aerofoil &tipFoil, float interp = 0.0f)
        : area(area), mid_span(mid_span), quater_chord(quater_chord), incidence(radians(incidenceDegrees)), rootFoil(rootFoil), tipFoil(tipFoil), interp(interp), mean_chord(mean_chord)
    {
    }

    //     Process the panel to calculate forces and moments.
    //     Args:
    //         state: Current state vector
    //         relative_velocity: The aircraft velocity relative to the air around it (in body axes, wind corrected)
    //         aircraft: Aircraft parameters including CG position
    //         world: The simulation world
    //         controls: Current control surface deflections
    //         sign: +1 for right wing, -1 for left wing
    //         forces - updated by adding in forces_body: [Fx, Fy, Fz] (N)
    //         moments - updated by adding in moments_body: [L, M, N] (N.m)
    void process(const StateVector<float> &state, const V3d<float> &relative_velocity, AircraftParameters &aircraft,
                 World &world, ControlInputs &controls, float sign, V3d<float> &forces, V3d<float> &moments)
    {
        // Get local airflow at panel due to angular velocity
        auto local_velocity = get_local_velocity(state, relative_velocity, sign);

        auto local_tas = local_velocity.TotalAirspeed();

        // Protection against very low airspeed (stall/spin conditions)
        float airspeed_factor = 1.0f;
        if (local_tas < MIN_AIRSPEED)
        {
            // Scale forces smoothly to zero as airspeed drops
            airspeed_factor = local_tas / MIN_AIRSPEED;
            local_tas = MIN_AIRSPEED; // Prevent division issues
        }

        auto local_alpha = local_velocity.AngleOfAttack();
        auto beta = local_velocity.SideslipAngle();

        auto aoa = local_alpha + incidence; // add geometric incidence angle

        // Hook: allow subclasses to modify AoA (e.g., aileron deflection)
        aoa = modify_aoa(aoa, controls, sign);

        // Dihedral effect: when slipping right (beta > 0), right wing sees increased AoA,
        // left wing sees decreased AoA. This creates restoring roll moment (Cl_beta).
        // The sign parameter differentiates right (+1) from left (-1) wing.
        // Simple linear model: delta_aoa = dihedral * beta * sign
        // Limited to prevent runaway at extreme sideslip
        const float MAX_DIHEDRAL_BETA = 0.35f;  // ~20 degrees
        auto beta_limited = clamp(beta, -MAX_DIHEDRAL_BETA, MAX_DIHEDRAL_BETA);
        aoa += aircraft.DihedralAngle() * beta_limited * sign;

        Aerofoil::Coefficients coeffs = coefficients_at(aoa);

        // Hook: allow subclasses to modify coefficients (e.g., spoiler lift reduction)
        modify_coefficients(coeffs, controls);

        auto Cl = coeffs.Cl;
        auto Cd = coeffs.Cd;
        auto Cm = coeffs.Cm;
        // Lift dependent drag
        auto Cdi = (Cl * Cl) / (float(PI) * aircraft.AR() * aircraft.Oswald());
        Cd += Cdi;

        auto q = 0.5f * world.air_density * local_tas * local_tas; // dynamic pressure
        auto L = Cl * q * area;
        auto D = Cd * q * area;

        auto M = Cm * q * area * mean_chord;

        // Hook: allow subclasses to add extra drag (e.g., deployed airbrakes)
        D += additional_drag(q, controls);

        // Transform from wind axes to body axes (rotation by angle of attack about Y)
        // Wind axes: -X is drag direction, -Z is lift direction
        // Body axes: X forward, Z down
        auto Fx = -D * cos(local_alpha) - L * sin(local_alpha); // drag backwards in S&L flight
        auto Fz = D * sin(local_alpha) - L * cos(local_alpha);  // lift is -ve Z in body axes

        // Apply low-airspeed scaling
        Fx *= airspeed_factor;
        Fz *= airspeed_factor;
        M *= airspeed_factor;

        // Clamp forces to prevent numerical instability
        Fx = clamp(safe_value(Fx), -MAX_PANEL_FORCE, MAX_PANEL_FORCE);
        Fz = clamp(safe_value(Fz), -MAX_PANEL_FORCE, MAX_PANEL_FORCE);
        M = clamp(safe_value(M), -MAX_PANEL_FORCE * 10, MAX_PANEL_FORCE * 10);

        auto forces_body = V3d<float>(Fx, 0.0, Fz); // drag in body X, side force 0, lift in body Z

        auto dist = quater_chord - aircraft.CG(); // calculate moments from c.g. not datum

        // Moments (about c.g.)
        // Roll moment due to lift at panel mid-span
        auto moments_body = V3d<float>(
            Fz * mid_span * sign, // roll moment
            M - Fz * dist,        // pitch moment about c.g.
            -Fx * mid_span * sign // yaw moment due to drag
        );

        forces += forces_body;
        moments += moments_body;
    }

    // --- Hook methods for subclasses to override ---

    // Hook: modify angle of attack based on control inputs. Override in subclasses.
    virtual float modify_aoa(float aoa, ControlInputs &controls, float sign)
    {
        return aoa;
    }

    // Hook: modify aerodynamic coefficients. Override in subclasses.
    virtual void modify_coefficients(Aerofoil::Coefficients &coeffs, ControlInputs &controls)
    {
    }

    // Hook: add additional drag based on control inputs. Override in subclasses.
    virtual float additional_drag(float q, ControlInputs &controls)
    {
        return 0.0;
    }

    // --- Utility methods ---


    // Calculate local velocity at panel due to angular velocity.
    // Retreating wing has reduced local airflow, advancing wing has increased local airflow.
    // Downgoing wing has increased local airflow, upgoing wing has reduced local airflow.
    // Args:
    //     state: Current state vector
    //     relative_velocity: Relative airframe velocity vector [u, v, w] in body frame relative to air-mass
    //     sign: +1 for right wing, -1 for left wing
    // Returns:
    //     local_airflow: Local airflow vector [u, v, w] at panel in body frame.
    V3d<float> get_local_velocity(const StateVector<float> &state, const V3d<float> &relative_velocity, float sign) const
    {
        auto p = state.angular_velocity()[0];
        auto r = state.angular_velocity()[2];

        // Change in z velocity. If rolling right, panel going down and Z increasing
        auto dz = p * mid_span * sign;
        // Change in x velocity. If yawing right, right panel retreating and X decreasing
        auto dx = -r * mid_span * sign;

        auto local_airflow = V3d<float>(relative_velocity[0] + dx, relative_velocity[1], relative_velocity[2] + dz);
        return local_airflow;
    }

    // Get lift, drag, moment coefficients at given angle of attack.
    // Args:
    //     aoa: Angle of attack in radians
    // Returns:
    //     (Cl, Cd, Cm) - lift, drag, moment coefficients
    Aerofoil::Coefficients coefficients_at(float aoa) const
    {

        auto rootFraction = 1.0f - interp;
        auto tipFraction = interp;

        auto root = rootFoil.coefficients_at(aoa);
        auto tip = tipFoil.coefficients_at(aoa);

        auto Cl = rootFraction * root.Cl + tipFraction * tip.Cl;
        auto Cd = rootFraction * root.Cd + tipFraction * tip.Cd;
        auto Cm = rootFraction * root.Cm + tipFraction * tip.Cm;
        return Aerofoil::Coefficients(Cl, Cd, Cm);
    }
};

// Panel with aileron control surface.
// Models aileron effects including:
// - Camber change affecting lift (via effective AoA shift)
// - Pitching moment change due to camber
// - Differential deflection (up vs down travel)
// - Profile drag from deflection
// Thin airfoil theory: A plain flap deflection changes:
// - Zero-lift angle: Δα_0 ≈ -ε * δ (ε = lift_effectiveness, typically 0.5-0.7)
// - Pitching moment: ΔCm = moment_coeff * δ (typically -0.3 to -0.5 per radian)
class AileronPanel : public Panel
{

    float _last_deflection = 0.0f; // store for drag and moment calculation

    float max_up;             // max deflection for up-going aileron
    float max_down;           // max deflection for down-going aileron
    float lift_effectiveness; // how much deflection changes effective AoA
    float moment_coeff;       // ΔCm per radian of deflection
    float profile_drag_coeff; // Drag coefficient per radian² of deflection.

public:
    // Args:
    //     lift_effectiveness: Fraction of deflection that acts as AoA change for lift.
    //                         Thin airfoil theory gives ~0.5-0.7 for typical aileron chord ratios.
    //     moment_coeff: Change in Cm per radian of deflection (negative = nose down for
    //                     trailing-edge-down deflection). Typical range -0.3 to -0.5.
    //     profile_drag_coeff: Drag coefficient per radian² of deflection.
    AileronPanel(float area, float mid_span, float quater_chord, float mean_chord, float incidenceDegrees,
                 Aerofoil &rootFoil, Aerofoil &tipFoil, float interp = 0.0,
                 float max_up_deg = 5.0, float max_down_deg = 5.0,
                 float lift_effectiveness = 0.6, float moment_coeff = -0.4,
                 float profile_drag_coeff = 0.01)
        : Panel(area, mid_span, quater_chord, mean_chord, incidenceDegrees, rootFoil, tipFoil, interp), max_up(radians(max_up_deg)) // max deflection for up-going aileron
          ,
          max_down(radians(max_down_deg)) // max deflection for down-going aileron
          ,
          lift_effectiveness(lift_effectiveness) // how much deflection changes effective AoA
          ,
          moment_coeff(moment_coeff) // ΔCm per radian of deflection
          ,
          profile_drag_coeff(profile_drag_coeff) // drag coefficient per radian² of deflection
    {
    }

    // Aileron deflection changes effective angle of attack (camber effect on lift).
    // Differential: up-going aileron can deflect more than down-going.
    // - controls.roll * sign > 0: aileron goes up (reduces AoA/lift)
    // - controls.roll * sign < 0: aileron goes down (increases AoA/lift)
    // The lift_effectiveness factor accounts for the fact that a plain flap
    // is less effective at changing lift than a pure AoA change.
    virtual float modify_aoa(float aoa, ControlInputs &controls, float sign)
    {
        auto command = controls.aileron * sign;
        auto deflection = (command >= 0) ?
                                         // Up-going aileron (reduces lift on this wing)
                              command * max_up
                                         :
                                         // Down-going aileron (increases lift on this wing)
                              command * max_down;

        _last_deflection = deflection;
        // Apply lift effectiveness - deflection is less effective than pure AoA change
        return aoa - deflection * lift_effectiveness;
    }

    // Modify pitching moment due to aileron camber change.
    // Trailing-edge-down deflection (positive δ) creates nose-down moment (negative ΔCm).
    // This is because the aft camber increase shifts the center of pressure rearward.
    virtual void modify_coefficients(Aerofoil::Coefficients &coeffs, ControlInputs &controls)
    {
        // Moment change due to camber: ΔCm = moment_coeff * δ
        // Note: _last_deflection is positive for up-aileron (reduces camber)
        // So we negate it: down deflection should give negative ΔCm
        auto delta_Cm = moment_coeff * (-_last_deflection);
        coeffs.Cm += delta_Cm;
    }

    virtual float additional_drag(float q, ControlInputs &controls)
    {
        // Deflected aileron adds profile drag proportional to deflection².
        //  Drag increment: Cd = k * δ²
        auto Cd_aileron = profile_drag_coeff * _last_deflection * _last_deflection;
        return Cd_aileron * q * area;
    }
};

// Panel with airbrake/spoiler control surface.
class AirbrakePanel : public Panel
{

    public:
    AirbrakePanel(float area, float mid_span, float quater_chord, float mean_chord, float incidenceDegrees,
                  Aerofoil &rootFoil, Aerofoil &tipFoil, float interp = 0.0)
        : Panel(area, mid_span, quater_chord, mean_chord, incidenceDegrees, rootFoil, tipFoil, interp)
    {
    }

    virtual void modify_coefficients(Aerofoil::Coefficients &coeffs, ControlInputs &controls)
    {
        // Airbrake deployment reduces lift coefficient.
        // Crude model: reduce lift proportionally to spoiler deployment
        auto lift_reduction = 0.8f * controls.spoiler;
        coeffs.Cl *= (1.0f - lift_reduction);
    }

    virtual float additional_drag(float q, ControlInputs &controls)
    {
        // Deployed airbrake adds significant drag.
        auto Cd_spoiler = 1.8f;          // flat plate drag coefficient
        auto spoiler_area = 0.3f * area; // airbrake is ~1/3 of panel area
        return Cd_spoiler * q * spoiler_area * controls.spoiler;
    }
};