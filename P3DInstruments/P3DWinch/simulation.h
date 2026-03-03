#pragma once
#include <cmath>
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
    float MAX_VELOCITY = 500.0;        // m/s - well beyond any realistic flight speed
    float MAX_ANGULAR_RATE = 20.0;     // rad/s - about 1150 deg/s
    float MAX_POSITION = 1000000.0;    // m - 1000 km


    float time_step = 0.01f;  // seconds
    float total_time = 0.0f;  // seconds
    World world;
    ASK21 aircraft;
    StateVector<float> state;
    ControlInputs controls;
    Model model;
    GroundContact ground_contact;
    Winch winch;
    std::map<std::string, float> winch_info;  // Diagnostic info from last winch calculation

    // Derived values for driving P3d    
    V3d<float> vel_dot; // Linear velocity derivative - linear acceleration
    V3d<float> omega_dot; // Angular velocity derivative - angular acceleration

    float clamp(float value, float min_val, float max_val);
    float safe_value(float value, float dflt = 0.0f);
    V3d<float> sanitize_velocity(float vx, float vy, float vz);
    V3d<float> sanitize_angular_velocity(float p, float q, float r);
    StateVector<float> rk4_step(const StateVector<float>& state, float dt);
    void calculate_forces_moments(const StateVector<float>& state, V3d<float>& forces, V3d<float>& moments);
    StateVector<float> state_derivative(const StateVector<float>& state, const V3d<float>& forces_body, const V3d<float>& moments_body);
    V3d<float> calculate_linear_acceleration(const StateVector<float>& state, const V3d<float>& forces_body);
    V3d<float> calculate_angular_acceleration(const StateVector<float>& state, const V3d<float>& moments_body);
    V3d<float> calculate_position_derivative(const StateVector<float>& state);
    V3d<float> apply_wind_to_state(const StateVector<float>&state, const V3d<float>&wind_earth);

public:

    // Acceleration values from last update.
    const V3d<float>& get_linear_acceleration() const { return vel_dot;}
    const V3d<float>& get_angular_acceleration() const { return omega_dot; }

    StateVector<float>& get_state() { return state; }

    void reset();
    StateVector<float> update(float dt, const ControlInputs& controls, const World& world);
    void setup_winch_launch(float winch_distance = 1500.0f,
        float max_tension = 9000.0f,
        float weak_link = 10000.0f);
    void engage_winch();
    void release_winch();
    
};