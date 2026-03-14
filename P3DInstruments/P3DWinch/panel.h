#pragma once

#include <cmath>
#include <algorithm>

#include "sim_types.h"
#include "aerofoil.h"
#include "v3d.h"
#include "state_vector.h"
#include "aircraft_params.h"
#include "world.h"
#include "control_inputs.h"
#include "local_math.h"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif


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
    const float MIN_AIRSPEED = 0.1f;

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

    inline NumberT clamp(NumberT value, NumberT min_val, NumberT max_val) const
    {
        // Clamp value to range [min_val, max_val].
        return std::max(min_val, std::min(max_val, value));
    }

    inline NumberT safe_value(NumberT value, NumberT dflt = 0.0f)
    {
        // Return default if value is NaN or Inf.
        return (std::isnan(value) || std::isinf(value)) ? dflt : value;
    }



public:
    Panel(float area, float mid_span, float quater_chord, float mean_chord, float incidenceDegrees, const Aerofoil& rootFoil, const Aerofoil& tipFoil, float interp = 0.0f);

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
    void process(const StateVector<NumberT>& state, const V3d<NumberT>& relative_velocity, AircraftParameters& aircraft,
        World& world, ControlInputs& controls, NumberT sign, V3d<NumberT>& forces, V3d<NumberT>& moments);
    
    // --- Hook methods for subclasses to override ---

    // Hook: modify angle of attack based on control inputs. Override in subclasses.
    virtual NumberT modify_aoa(NumberT aoa, ControlInputs& controls, NumberT sign);

    // Hook: modify aerodynamic coefficients. Override in subclasses.
    virtual void modify_coefficients(Aerofoil::Coefficients& coeffs, ControlInputs& controls);

    // Hook: add additional drag based on control inputs. Override in subclasses.
    virtual NumberT additional_drag(NumberT q, ControlInputs& controls);

    // --- Utility methods ---

    // Calculate local velocity at panel due to angular velocity.
    V3d<NumberT> get_local_velocity(const StateVector<NumberT>& state, const V3d<NumberT>& relative_velocity, NumberT sign) const;

    // Get lift, drag, moment coefficients at given angle of attack.
    Aerofoil::Coefficients coefficients_at(NumberT aoa) const;
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

    NumberT _last_deflection = 0.0f; // store for drag and moment calculation

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
        Aerofoil& rootFoil, Aerofoil& tipFoil, float interp = 0.0,
        float max_up_deg = 5.0, float max_down_deg = 5.0,
        float lift_effectiveness = 0.6, float moment_coeff = -0.4,
        float profile_drag_coeff = 0.01);

    // Aileron deflection changes effective angle of attack (camber effect on lift).
    virtual NumberT modify_aoa(NumberT aoa, ControlInputs& controls, NumberT sign);

    // Modify pitching moment due to aileron camber change.
    virtual void modify_coefficients(Aerofoil::Coefficients& coeffs, ControlInputs& controls);

    virtual NumberT additional_drag(NumberT q, ControlInputs& controls);
};

// Panel with airbrake/spoiler control surface.
class AirbrakePanel : public Panel
{

    public:
        AirbrakePanel(float area, float mid_span, float quater_chord, float mean_chord, float incidenceDegrees,
            Aerofoil& rootFoil, Aerofoil& tipFoil, float interp = 0.0);

        virtual void modify_coefficients(Aerofoil::Coefficients& coeffs, ControlInputs& controls);

        virtual NumberT additional_drag(NumberT q, ControlInputs& controls);
};