#pragma once
#include <cmath>
#include "sim_types.h"
#include "world.h"
#include "state_vector.h"
#include "aircraft_params.h"
#include "control_inputs.h"
#include "model.h"
#include "ask21.h"
#include "ground_contact.h"
#include "winch.h"





//  Main simulation class that integrates all components and runs the simulation loop.

class Simulation {

    // State limits to prevent numerical divergence
    NumberT MAX_VELOCITY = 500.0;        // m/s - well beyond any realistic flight speed
    NumberT MAX_ANGULAR_RATE = 20.0;     // rad/s - about 1150 deg/s
    NumberT MAX_POSITION = 1000000.0;    // m - 1000 km


    NumberT time_step = 0.01f;  // seconds
    NumberT total_time = 0.0f;  // seconds

    World world;
    ASK21 aircraft;
    StateVector<NumberT> state;
    ControlInputs controls;
    Model model;
    GroundContact ground_contact;
    Winch winch;

    // Derived values for driving P3d    
    V3d<NumberT> vel_dot; // Linear velocity derivative - linear acceleration
    V3d<NumberT> omega_dot; // Angular velocity derivative - angular acceleration

    NumberT rollBias = 0.0; // adds roll bias force-  primarily for simulating wing drops

    NumberT clamp(NumberT value, NumberT min_val, NumberT max_val);
    NumberT safe_value(NumberT value, NumberT dflt = 0.0f);
    V3d<NumberT> sanitize_velocity(NumberT vx, NumberT vy, NumberT vz);
    V3d<NumberT> sanitize_angular_velocity(NumberT p, NumberT q, NumberT r);
    StateVector<NumberT> rk4_step(const StateVector<NumberT>& state, NumberT dt);
    void calculate_forces_moments(const StateVector<NumberT>& state, V3d<NumberT>& forces, V3d<NumberT>& moments);
    StateVector<NumberT> state_derivative(const StateVector<NumberT>& state, const V3d<NumberT>& forces_body, const V3d<NumberT>& moments_body);
    V3d<NumberT> calculate_linear_acceleration(const StateVector<NumberT>& state, const V3d<NumberT>& forces_body);
    V3d<NumberT> calculate_angular_acceleration(const StateVector<NumberT>& state, const V3d<NumberT>& moments_body);
    V3d<NumberT> calculate_position_derivative(const StateVector<NumberT>& state);
    V3d<NumberT> apply_wind_to_state(const StateVector<NumberT>&state, const V3d<NumberT>&wind_earth);

public:

    // Acceleration values from last update.
    const V3d<NumberT>& get_linear_acceleration() const { return vel_dot;}
    const V3d<NumberT>& get_angular_acceleration() const { return omega_dot; }

    // Current state vector
    StateVector<NumberT>& get_state() { return state; }

    // reset and upate state.
    void reset();
    StateVector<NumberT> update(NumberT dt, const ControlInputs& controls, const World& world);


    void setup_winch_launch(float winch_distance = 1500.0f,
        float weak_link = 10000.0f);
    void setup_winch_launch_at(const V3d<float>& winch_pos, float weak_link = 10000.0f);
    void engage_winch();
    void release_winch();

    // Winch throttle passthrough
    void set_winch_throttle(float t);
    void clear_winch_throttle_override();
    float get_winch_throttle() const;
    float get_winch_cable_angle() const;
    bool  is_winch_engaged() const;
    NumberT get_winch_tension() const;
    float get_winch_engine_rpm() const;
    int   get_winch_gear() const;

    // Spin-kit
    void set_spin_kit(float kg) {
        aircraft.set_spin_kit(kg);
    }


    // ground contact
    bool is_on_ground() {
        return ground_contact.is_on_ground();
    }
    // and height of aircraft datum off the main wheel.
    float zOffset() { return aircraft.zOffset(); }

    // Sets a bias to roll in Nm
    void setRollBias(NumberT bias) { rollBias = bias; }
};