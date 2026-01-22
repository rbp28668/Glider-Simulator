
#pragma once

#include <cmath>
#include <algorithm>

#include "ask21.h"
#include "state_vector.h"
#include "control_inputs.h"
#include "world.h"
#include "v3d.h"



//   This is the aerodynamic model for the simulation.

class Model{

    // Minimum airspeed for aerodynamic calculations (m/s)
    float MIN_AIRSPEED = 1.0;

    // Maximum total force/moment to prevent numerical overflow
    float MAX_FORCE = 100000.0;
    float MAX_MOMENT = 500000.0;

    

    float clamp(float value, float min_val, float max_val){
        // Clamp value to range [min_val, max_val].
        return std::max(min_val, std::min(max_val, value));
    }


    float safe_value(float value, float dflt = 0.0){
        // Return default if value is NaN or Inf.
        return isnan(value) || isinf(value) ? dflt : value;
    }
        
    public:

    Model() {}
    ~Model() {}


    // Calculate aerodynamic forces and moments
    // Args:
    //     state: Current state vector
    //     aircraft: The aircraft model
    //     control_inputs: Current control surface deflections
    //     world: The simulation world
    //     relative_velocity: The aircraft velocity relative to the air-mass (in body axes, wind corrected)
    // Returns:
    //     forces_body: [Fx, Fy, Fz] (N)
    //     moments_body: [L, M, N] (N·m)

    void calculate_aerodynamics(const StateVector<float>& state, const ASK21& aircraft, const ControlInputs& control_inputs, const World& world, const V3d<float>& relative_velocity,
    V3d<float>& forces, V3d<float>& moments){


        V3d<float> forces_body ;
        V3d<float> moments_body ;

        //Wings
        for(auto iter = aircraft.wing.begin(); iter != aircraft.wing.end(); ++iter){
            Panel* panel = *iter;
            // Right hand side            
            panel->process(state, relative_velocity, (AircraftParameters&)aircraft, (World&)world, (ControlInputs&)control_inputs, 1.0, forces_body, moments_body);
            // Left hand side
            panel->process(state, relative_velocity, (AircraftParameters&)aircraft, (World&)world, (ControlInputs&)control_inputs, -1.0, forces_body, moments_body);
        }

        //Tailplane
        tailplane_forces(state, aircraft, control_inputs,world, relative_velocity,  forces_body, moments_body) ;
        
        //Fin
        fin_forces(state, aircraft, control_inputs, world, relative_velocity, forces_body, moments_body) ;

        // Explicit roll damping (Clp effect)
        // Wing panels provide some damping via local velocity, but add explicit term
        // for robustness at high rates. Clp is typically -0.4 to -0.5 for gliders.
        auto roll_rate = state.angular_velocity()[0];
        auto tas = relative_velocity.TotalAirspeed();
        if (tas > MIN_AIRSPEED) {
            auto q = 0.5f * world.air_density * tas * tas;
            float Clp = -0.4f;  // roll damping derivative
            float wing_span = 17.0f;  // ASK-21 span (m)
            float wing_area = wing_span * wing_span / aircraft.AR();  // S = b²/AR
            // Damping moment: Clp * (p * b/2V) * q * S * b
            auto roll_damping = Clp * roll_rate * q * wing_area * wing_span / (2.0f * tas);
            moments_body[0] += roll_damping;

            // Adverse yaw from aileron (Cn_δa effect)
            // Down-going aileron increases lift and induced drag, up-going decreases it
            // This drag differential creates yaw opposite to roll direction
            float Cn_da = -0.01f;  // adverse yaw derivative (per unit roll command)
            auto adverse_yaw = Cn_da * control_inputs.aileron * q * wing_area * wing_span;
            moments_body[2] += adverse_yaw;
        }

        // Fuselage drag approximation
        // ASK-21 fuselage equivalent flat plate area ~0.025 m² (typical for training glider)
        // This includes fuselage, canopy, wing-fuselage interference, control surface gaps, etc.
        float fuselage_Cd_S = 0.025f;  // m² equivalent flat plate area
        if (tas > MIN_AIRSPEED) {
            auto q = 0.5f * world.air_density * tas * tas;
            auto fuselage_drag = fuselage_Cd_S * q;
            forces_body[0] -= fuselage_drag;  // drag acts backward (-X direction)
        }

        // Fuselage side-area drag (critical for sideslip dynamics)
        // ASK-21 fuselage side projected area ~5 m², Cd ~1.0 for bluff body in crossflow
        // This creates drag proportional to sin²(beta), opposing sideslip
        float fuselage_side_Cd_S = 5.0f;  // m² effective side area * Cd
        if (tas > MIN_AIRSPEED) {
            auto q = 0.5f * world.air_density * tas * tas;
            auto beta = relative_velocity.SideslipAngle();
            auto sin_beta = sin(beta);
            // Side drag force opposes sideslip velocity (acts in -Y when v > 0)
            auto side_drag = fuselage_side_Cd_S * q * sin_beta * fabs(sin_beta);
            forces_body[1] -= side_drag;  // opposes sideslip
        }

        // TODO - Cm_beta : pitch down with sideslip

        // Sanitize and clamp final forces/moments to prevent numerical overflow
        forces[0] = clamp(safe_value(forces_body[0]), -MAX_FORCE, MAX_FORCE);
        forces[1] = clamp(safe_value(forces_body[1]), -MAX_FORCE, MAX_FORCE);
        forces[2] = clamp(safe_value(forces_body[2]), -MAX_FORCE, MAX_FORCE);
        moments[0] = clamp(safe_value(moments_body[0]), -MAX_MOMENT, MAX_MOMENT);
        moments[1] = clamp(safe_value(moments_body[1]), -MAX_MOMENT, MAX_MOMENT);
        moments[2] = clamp(safe_value(moments_body[2]), -MAX_MOMENT, MAX_MOMENT);

        return;
    }

    // Calculate tailplane aerodynamic forces
    // Args:
    //     state: Current state vector
    //     aircraft: The aircraft model
    //     relative_velocity: The aircraft velocity relative to the air-mass (in body axes, wind corrected)
    //     world: The simulation world
    
    // Returns:
    //     nothing - updates forces and moments passed by reference

    void tailplane_forces(const StateVector<float>& state, const ASK21& aircraft, const ControlInputs& control_inputs, const World& world, const V3d<float>& relative_velocity,
    V3d<float>& forces, V3d<float>& moments){


        //Allow for pitch rate to change airflow at tail.  Pitching up then tail going down (+ve direction)
        auto dist = aircraft.tailplane_quarter_chord - aircraft.cg;  // distance of tailplane A/C from c of g  (-ve as behind)
        auto pitch_rate = state.angular_velocity()[1];               // +ve pitch rate -> nose up so tail down (+ve z dirn)
        auto vz_pitch = pitch_rate * -dist;

        auto tailplane_velocity = V3d<float>( 
            relative_velocity[0],  // u - velocity forward
            relative_velocity[1],  // v - velocity to right
            relative_velocity[2] + vz_pitch); // add in extra vertical velocity do to pitch rate
        

        auto tas = tailplane_velocity.TotalAirspeed();

        // Low airspeed protection
        if (tas < MIN_AIRSPEED)
            return; // without making changes to forces/moments

        auto aoa = tailplane_velocity.AngleOfAttack() + aircraft.tailplane_incidence;
        aoa -= control_inputs.elevator * radians(10.0f);  // TODO properly - elevator effect

        auto coeffs = aircraft.tailplane.coefficients_at(aoa);
        auto Cl = coeffs.Cl;
        auto Cd = coeffs.Cd;
        // TODO - Cm

        auto q = 0.5f * world.air_density * tas * tas;
        auto tp_L = Cl * q * aircraft.tailplane_area;
        auto tp_D = Cd * q * aircraft.tailplane_area;
        float M = 0;  // TODO - CM

        // Transform from wind axes to body axes (rotation by angle of attack about Y)
        auto D = -tp_D * cos(aoa) - tp_L * sin(aoa);  // drag backwards
        auto L =  tp_D * sin(aoa) - tp_L * cos(aoa);  // lift up is -ve Z in body axes

        //return (safe_value(L), safe_value(D), safe_value(M))

        //       tp_L, tp_D, tp_M = self.tailplane_forces(state, aircraft, relative_velocity, control_inputs, world) 
        forces[0] += D;     // drag in body X
        forces[2] += L;     // lift in body z
        // Moments (about c.g.)
        moments[1] += M - L * dist;  // pitch moment due to lift at tailplane quarter chord (- sign as dist is -ve as behind c.g.)
        }

  void fin_forces(const StateVector<float>& state, const ASK21& aircraft, const ControlInputs& control_inputs, const World& world, const V3d<float>& relative_velocity,
    V3d<float>& forces, V3d<float>& moments){
        // Distance from CG to fin (positive = aft of CG)
        auto fin_arm = aircraft.cg - aircraft.fin_quarter_chord;  // positive value (~4.78m)

        // Yaw rate effect on fin airflow
        // When yawing right (r > 0), fin at x<0 moves left, experiencing "headwind" from left
        // This reduces the v-component of airflow at fin, creating restoring moment
        auto yaw_rate = state.angular_velocity()[2];
        auto fin_airflow = V3d<float>(relative_velocity[0],
                       relative_velocity[1] + yaw_rate * aircraft.fin_quarter_chord,
                       relative_velocity[2]);

        auto fin_tas = fin_airflow.TotalAirspeed();

        // Low airspeed protection
        if (fin_tas < MIN_AIRSPEED)
            return; // without making changes to forces/moments

        auto fin_aoa = fin_airflow.SideslipAngle();

        // Rudder effect
        fin_aoa += control_inputs.rudder * radians(15.0f);  // max 15 degrees aoa change due to rudder

        auto coeffs = aircraft.fin.coefficients_at(fin_aoa);
        auto fin_Cl = coeffs.Cl;
        auto fin_Cd = coeffs.Cd;
        // TODO - Cn
        auto fin_q = 0.5f * world.air_density * fin_tas * fin_tas;
        auto fin_L = fin_Cl * fin_q * aircraft.fin_area;
        auto fin_D = fin_Cd * fin_q * aircraft.fin_area;

        // Additional yaw damping (Cnr effect) - opposes yaw rate
        // This represents damping from fuselage, fin boundary layer, etc.
        // Negative sign ensures moment opposes yaw rate (damping, not divergence)
        float Cnr = 0.05f;  // yaw damping coefficient
        float yaw_damping = -Cnr * yaw_rate * fin_q * aircraft.fin_area * fin_arm;

        // Transform from wind axes to body axes
        auto D = -fin_D * cos(fin_aoa) - fin_L * sin(fin_aoa);  // drag backwards
        auto L =  fin_D * sin(fin_aoa) - fin_L * cos(fin_aoa);  // side force (fin "lift")

        forces[0] += D;     // drag in body X
        forces[1] += L;     // side force already in body frame (negative = left)

        // Moments (about c.g.)
        // Yaw moment = position_x × Fy = dist × L
        auto dist = aircraft.fin_quarter_chord - aircraft.cg;
        moments[2] += L * dist;  // yaw moment from side force at fin
        moments[2] += yaw_damping;  // explicit yaw damping
        }

    // @staticmethod
    // def add(acc: list[float], v1: V3d, v2: V3d) -> None:
    //     acc[0] += (v1[0] + v2[0])     // drag in body X
    //     acc[1] += (v1[1] + v2[1])    // side force in body Y
    //     acc[2] += (v1[2] + v2[2])    // lift in body z

    
        };