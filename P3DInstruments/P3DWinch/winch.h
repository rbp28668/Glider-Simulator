
#pragma once

#include <cmath>
#include <string>
#include <map>
#include <algorithm>
#include "state_vector.h"
#include "v3d.h"
#include "quaternion.h"
#include "local_math.h"


// Winch launch simulation for glider.

// Models a ground-based winch that reels in a cable attached to the glider's
// winch hook, providing the force for a winch launch.

// Features:
// - Cable tension based on winch power and drum speed
// - Automatic back-release if cable pulls backwards relative to glider
// - Weak link simulation (optional)
// - Cable runs out detection



// Winch launch simulation.
// The winch is positioned at a fixed location on the ground and reels in
// a cable attached to the glider's winch hook. The cable applies a tension
// force along its length from the hook towards the winch.
// Back-release: If the cable direction has a negative X component in the
// glider's body frame (pulling backwards), the cable automatically releases.
class Winch{

    V3d<float> winch_position;  // (X, Y, Z) position of winch drum in earth frame (m)
    float max_tension;          // Maximum cable tension from winch power (N)   
    float weak_link;            // Tension at which weak link breaks (N)
    float cable_length;         // Total cable length on drum (m)
    bool engaged;               // Is the winch cable currently engaged
    float cable_out;           // Length of cable paid out (m)  
    float tension;              // Current cable tension (N)
    std::string release_reason; // Why cable was released
    float cable_stiffness;      // N/m - cable elasticity
    float cable_damping;        // N.s/m - cable damping    
    float drum_speed;           // Target reel-in speed (m/s)
    float drum_power;           // Winch power (W) - ~450 HP
public:



        // Initialize winch.
        // Args:
        //     winch_position: (X, Y, Z) position of winch drum in earth frame (m)
        //                    Default 1000m ahead (north), on ground
        //     max_tension: Maximum cable tension from winch power (N)
        //                 Default 9000N (~1.3x glider weight for good climb)
        //     weak_link: Tension at which weak link breaks (N)
        //               Default 10000N
        //     cable_length: Total cable length on drum (m)
        //                  Default 1500m
        Winch( const V3d<float>& winch_position = V3d<float>(1000.0, 0.0, 0.0),
               float max_tension = 9000.0,
               float weak_link = 10000.0,
               float cable_length = 1500.0){

            this->winch_position = winch_position;
            this->max_tension = max_tension;
            this->weak_link = weak_link;
            this->cable_length = cable_length;
    
            // State
            engaged = false;
            cable_out = 0.0;  // Length of cable paid out (m)
            tension = 0.0;    // Current cable tension (N)
            release_reason = "";  // Why cable was released
    
            // Cable properties
            cable_stiffness = 50000.0;  // N/m - cable elasticity
            cable_damping = 1000.0;     // N.s/m - cable damping
    
            // Winch drum model
            drum_speed = 30.0;  // Target reel-in speed (m/s)
            drum_power = 335000.0;  // Winch power (W) - ~450 HP
        }


    // Engage the winch cable.
    // Args:
    //     initial_cable_out: Initial cable length paid out (m)
    //                         If None, calculated from glider position
    void  engage(float initial_cable_out = -1.0){
        engaged = true;
        release_reason = "";
        if (initial_cable_out > 0)
            cable_out = initial_cable_out;
    }

    // Release the cable.
    //     Args:
    //         reason: Why the cable was released
    void release(const std::string& reason = "manual"){
        engaged = false;
        tension = 0.0;
        release_reason = reason;
    }


    //     Calculate winch cable forces and moments.
    // Args:
    //     state: Current aircraft state
    //     hook_position_body: Winch hook position in body frame (m)
    // Returns:
    //     forces_body: Cable force in body frame (Fx, Fy, Fz) in N
    //     moments_body: Moment about CG in body frame (L, M, N) in N.m
    //     info: Dictionary with diagnostic information
    void calculate_forces(const StateVector<float>& state, const V3d<float>& hook_position_body,
                         V3d<float>& forces_body, V3d<float>& moments_body ){

        // No force if not engaged
        if (!engaged) return;

        // Get hook position in earth frame
        auto orientation = state.orientation();
        auto hook_earth_offset = orientation.rotate_vector(hook_position_body);
        auto aircraft_pos = state.position();

        auto hook_earth = V3d<float>(
            aircraft_pos[0] + hook_earth_offset[0],
            aircraft_pos[1] + hook_earth_offset[1],
            aircraft_pos[2] + hook_earth_offset[2]
        );

        // Vector from hook to winch (cable direction)
        auto cable_vec = V3d<float>(
            winch_position[0] - hook_earth[0],
            winch_position[1] - hook_earth[1],
            winch_position[2] - hook_earth[2]
        );

        // Cable length (distance from hook to winch)
        auto cable_distance = sqrt(cable_vec[0]*cable_vec[0] 
            + cable_vec[1]*cable_vec[1] 
            + cable_vec[2]*cable_vec[2] );

        if (cable_distance < 100.0){
            // Too close to winch, release
            release("cable_run_out");
            return;
        }

        // Unit vector along cable (from hook towards winch)
        auto cable_unit = V3d<float>(
            cable_vec[0] / cable_distance,
            cable_vec[1] / cable_distance,
            cable_vec[2] / cable_distance
        );

        // Transform cable direction to body frame
        auto cable_body = orientation.rotate_vector_inverse(cable_unit);

        // Check for back-release: cable pulling backwards (negative X in body frame)
        if (cable_body[0] < 0){
            release("back_release");
            return;
        }
        // // Calculate cable angle from horizontal (for info)
        // auto horizontal_dist = sqrt(cable_vec[0]*cable_vec[0] + cable_vec[1]*cable_vec[1]);
        // if (horizontal_dist > 0.1){
        //     auto cable_angle = acos(std::min(1.0f, horizontal_dist / cable_distance));
        // }

        // Update cable out length
        cable_out = cable_distance;

        // Check if cable has run out - not that this EVER happens.
        if (cable_out > cable_length) {
            release("cable_run_out");
            return;
        }

        // Calculate tension using power-limited winch model
        // Power = Force * velocity
        // At the hook, velocity component along cable determines power delivery

        // Get hook velocity in earth frame
        auto hook_vel_body = get_hook_velocity(state, hook_position_body);
        auto hook_vel_earth = orientation.rotate_vector(hook_vel_body);

        // Velocity component along cable (positive = towards winch = good)
        auto v_cable = (
            hook_vel_earth[0] * cable_unit[0] +
            hook_vel_earth[1] * cable_unit[1] +
            hook_vel_earth[2] * cable_unit[2]
        );

        // Winch drum tries to reel in at drum_speed
        // Tension is based on difference between drum speed and actual cable speed
        auto speed_error = drum_speed - v_cable;

        // Simple tension model: tension proportional to speed error, limited by power
        if(speed_error > 0){
            // Cable moving slower than drum wants - apply tension
            // Power limited: T * v <= Power, so T <= Power / v
            auto power_limited_tension = (v_cable > 1.0) ? (drum_power / v_cable) : max_tension;

            tension = std::min(max_tension, power_limited_tension);

        }else{
            // Cable moving faster than drum - minimal tension (cable slack)
            tension = 100.0;  // Small tension to keep cable taut
        }

        // Check weak link
        if(tension > weak_link){
            release("weak_link");
            return;
        }

        // Apply force along cable direction (in earth frame, towards winch)
        auto force_earth = V3d<float>(
            tension * cable_unit[0],
            tension * cable_unit[1],
            tension * cable_unit[2]
        );

        // Transform force to body frame
        forces_body = orientation.rotate_vector_inverse(force_earth);

        // Calculate moment about CG
        // Hook position relative to CG (assuming CG at datum for now)
        // The simulation will need to pass CG offset, but for now use hook_position_body
        auto arm = hook_position_body;

        // Moment = arm x force
        moments_body = V3d<float>(
            arm[1] * forces_body[2] - arm[2] * forces_body[1],
            arm[2] * forces_body[0] - arm[0] * forces_body[2],
            arm[0] * forces_body[1] - arm[1] * forces_body[0]
        );

        return; // with forces_body and moments_body set
    }

    // Calculate velocity of hook point in body frame.
    // V_hook = V_cg + omega x r_hook
    V3d<float> get_hook_velocity(const StateVector<float>& state, const V3d<float>& hook_body) {
        auto u = state.velocity()[0];
        auto v = state.velocity()[1];
        auto w = state.velocity()[2];
        auto p = state.angular_velocity()[0];
        auto q = state.angular_velocity()[1];
        auto r = state.angular_velocity()[2];


        // omega x r (cross product)
        V3d<float>omega_cross_r = V3d<float>(
            q * hook_body[2] - r * hook_body[1],
            r * hook_body[0] - p * hook_body[2],
            p * hook_body[1] - q * hook_body[0]
        );

        return V3d<float>(
            u + omega_cross_r[0],
            v + omega_cross_r[1],
            w + omega_cross_r[2]
        );

    }



};
