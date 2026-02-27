
#include "simulation.h"




//Clamp value to range [min_val, max_val].
float Simulation::clamp(float value, float min_val, float max_val) {
    return std::max(min_val, std::min(max_val, value));
}

// Return default if value is NaN or Inf.
float Simulation::safe_value(float value, float dflt) {
    return  (isnan(value) || isinf(value)) ? dflt : value;
}


//Clamp velocity components to safe range.
V3d<float> Simulation::sanitize_velocity(float vx, float vy, float vz) {
    return V3d<float>(
        clamp(safe_value(vx), -MAX_VELOCITY, MAX_VELOCITY),
        clamp(safe_value(vy), -MAX_VELOCITY, MAX_VELOCITY),
        clamp(safe_value(vz), -MAX_VELOCITY, MAX_VELOCITY)
        );
}

//Clamp angular velocity components to safe range.
V3d<float> Simulation::sanitize_angular_velocity(float p, float q, float r) {
    return V3d<float>(
        clamp(safe_value(p), -MAX_ANGULAR_RATE, MAX_ANGULAR_RATE),
        clamp(safe_value(q), -MAX_ANGULAR_RATE, MAX_ANGULAR_RATE),
        clamp(safe_value(r), -MAX_ANGULAR_RATE, MAX_ANGULAR_RATE)
        );
}


// Reset simulation to initial state
void Simulation::reset() {
    total_time = 0.0;
    state = StateVector<float>();
}

// def update(self, dt : float) -> StateVector:
//     self.total_time += dt
//     print(f"DT: {dt}")

//     if self.use_rk4:
//         return self.update_rk4(dt)
//     else:
//         return self.update_simple(dt)


// def update_simple(self, dt : float) -> StateVector:
//     state = self.state
//     // Get wind
//     wind_earth = self.world.get_wind_vector(state.position(), self.total_time)


//     // Calculate airspeed with wind
//     V_air_body = self.apply_wind_to_state(state, wind_earth)

//     // Calculate aerodynamics
//     forces_body, moments_body = self.model.calculate_aerodynamics(
//         state, self.aircraft, self.controls, self.world, V_air_body
//     )

//     // Get current velocity and angular velocity for Coriolis terms
//     u, v, w = state.velocity()
//     p, q, r = state.angular_velocity()

//     // Linear acceleration: F/m + gravity + Coriolis terms
//     // In rotating body frame: a_body = F/m + g_body + ω × v_body
//     // Coriolis: (r*v - q*w, p*w - r*u, q*u - p*v)
//     acc_x = forces_body[0] / self.aircraft.mass + (r*v - q*w)
//     acc_y = forces_body[1] / self.aircraft.mass + (p*w - r*u)
//     acc_z = forces_body[2] / self.aircraft.mass + (q*u - p*v)

//     orientation = state.orientation()

//     // Gravity in body frame
//     g = 9.81
//     g_earth = (0, 0, g) // +ve down
//     g_body = quaternion_rotate_vector_inverse(orientation, g_earth)

//     acc_x += g_body[0]
//     acc_y += g_body[1]
//     acc_z += g_body[2]

//     // Update velocity: v_new = v_old + a * dt
//     vx = u + acc_x * dt
//     vy = v + acc_y * dt
//     vz = w + acc_z * dt

//     // Update position using OLD velocity (standard forward Euler)
//     v_world = quaternion_rotate_vector(orientation, (u, v, w))  // body to world frame
//     position = state.position()
//     px = position[0] + v_world[0] * dt
//     py = position[1] + v_world[1] * dt
//     pz = position[2] + v_world[2] * dt

//     // Moments of inertia
//     Ixx = self.aircraft.Ixx
//     Iyy = self.aircraft.Iyy
//     Izz = self.aircraft.Izz

//     // Angular acceleration with gyroscopic coupling
//     // Using Euler's equations: I * ω̇ = M - ω × (I * ω)
//     dp = (moments_body[0] + (Iyy - Izz) * q * r) / Ixx
//     dq = (moments_body[1] + (Izz - Ixx) * r * p) / Iyy
//     dr = (moments_body[2] + (Ixx - Iyy) * p * q) / Izz

//     // Update angular velocity
//     av_roll = p + dp * dt
//     av_pitch = q + dq * dt
//     av_yaw = r + dr * dt

//     // Note: Artificial damping removed - natural aerodynamic damping comes from:
//     // - Tail seeing different AoA due to pitch rate (modeled in tailplane_forces)
//     // - For proper damping, add Cmq derivative term to pitching moment

//     // Constrain to pitch-only motion if set
//     if(self.pitch_only):
//         av_yaw = 0
//         av_roll = 0
//         vy = 0

//     // Orientation update via quaternion derivative (using NEW angular velocity)
//     quat = state.orientation()
//     quat_dot = quaternion_derivative(quat, (av_roll, av_pitch, av_yaw))
//     qw = quat[0] + quat_dot[0] * dt
//     qx = quat[1] + quat_dot[1] * dt
//     qy = quat[2] + quat_dot[2] * dt
//     qz = quat[3] + quat_dot[3] * dt

//     // Update position using euler angles for DEBUG
//     // Note - provides same behaviour
//     // roll, pitch, yaw = quaternion_to_euler(quat[0],quat[1], quat[2], quat[3])
//     // pitch += angular_velocity[1] * dt
//     // quat = euler_to_quaternion(roll, pitch, yaw)
//     // qw = quat[0] 
//     // qx = quat[1] 
//     // qy = quat[2] 
//     // qz = quat[3] 

//     new_state = StateVector()

//     // Sanitize position (allow large range but catch NaN/Inf)
//     new_state.set_position((
//         clamp(safe_value(px), -MAX_POSITION, MAX_POSITION),
//         clamp(safe_value(py), -MAX_POSITION, MAX_POSITION),
//         clamp(safe_value(pz), -MAX_POSITION, MAX_POSITION)
//     ))

//     // Sanitize velocity
//     new_state.set_velocity(sanitize_velocity(vx, vy, vz))

//     // Sanitize angular velocity
//     new_state.set_angular_velocity(sanitize_angular_velocity(av_roll, av_pitch, av_yaw))

//     // Sanitize quaternion (normalize handles most issues, but check for NaN)
//     quat_safe = (
//         safe_value(qw, 1.0),
//         safe_value(qx, 0.0),
//         safe_value(qy, 0.0),
//         safe_value(qz, 0.0)
//     )
//     new_state.set_orientation(quaternion_normalize(quat_safe))
//     new_state.set_forces_moments(forces_body, moments_body)

//     self.state = new_state

//     return new_state


// Update aircraft state based on physics, control inputs, and world conditions
// Uses Runge-Kutta 4th order integration
StateVector<float> Simulation::update(float dt, const ControlInputs& controls, const World& world) {

    // Update control positions & world view
    this->controls = controls;
    this->world = world;


    //Advance simulation by one time step
    total_time += dt;

    // Integration
    state = rk4_step(state, dt);

    return state;
}

//     4th order Runge-Kutta integration step
// Args:
//     state: Current state vector
//     dt: Time step (s)
//     forces_moments_func: Function that returns (forces, moments)
//     mass: Aircraft mass (kg)
//     inertia: Inertia tensor
// Returns:
//     new_state: Updated state vector

StateVector<float> Simulation::rk4_step(const StateVector<float>& state, float dt) {

    // k1
    V3d<float>forces1, moments1;
    calculate_forces_moments(state, forces1, moments1);
    auto k1 = state_derivative(state, forces1, moments1);

    // k2
    auto state2 = state.offset(k1, 0.5f * dt); //[state[i] + 0.5*dt*k1[i] for i in range(13)]
    state2.normalize_orientation();
    V3d<float>forces2, moments2;
    calculate_forces_moments(state2, forces2, moments2);
    auto k2 = state_derivative(state2, forces2, moments2);

    // k3
    auto state3 = state.offset(k2, 0.5f * dt); //[state[i] + 0.5*dt*k2[i] for i in range(13)]
    state3.normalize_orientation();
    V3d<float>forces3, moments3;
    calculate_forces_moments(state3, forces3, moments3);
    auto k3 = state_derivative(state3, forces3, moments3);

    // k4
    auto state4 = state.offset(k3, dt); //[state[i] + dt*k3[i] for i in range(13)]
    state4.normalize_orientation();
    V3d<float>forces4, moments4;
    calculate_forces_moments(state4, forces4, moments4);
    auto k4 = state_derivative(state4, forces4, moments4);

    // Combine & normalize
    auto new_state = state.rk4_sum(k1, k2, k3, k4, dt);
    new_state.normalize_orientation();

    return new_state;
}



// Calculate total forces and moments (aerodynamic + ground contact + winch)
// Args:
//     state: Current state vector
// Returns:
//     forces_body: [Fx, Fy, Fz] (N)
//     moments_body: [L, M, N] (N·m)
void Simulation::calculate_forces_moments(const StateVector<float>& state, V3d<float>& forces, V3d<float>& moments) {

    // Get wind
    auto wind_earth = world.get_wind_vector(state.position(), total_time);

    // Calculate airspeed with wind
    auto V_air_body = apply_wind_to_state(state, wind_earth);

    // Calculate aerodynamics
    V3d<float>aero_forces, aero_moments;
    model.calculate_aerodynamics(state, aircraft, controls, world, V_air_body, aero_forces, aero_moments);

    // Calculate ground contact forces
    V3d<float>ground_forces, ground_moments;
    std::vector<ContactResult> results;
    ground_contact.calculate_ground_forces(state, aircraft.contact_points, world, aircraft.cg, ground_forces, ground_moments, results);


    // Calculate winch forces
    V3d<float>winch_forces, winch_moments;
    auto hook_body = aircraft.winch_hook;
    winch.calculate_forces(state, hook_body, winch_forces, winch_moments);

    // Accumulate total forces and moments
    forces = V3d<float>(
        aero_forces[0] + ground_forces[0] + winch_forces[0],
        aero_forces[1] + ground_forces[1] + winch_forces[1],
        aero_forces[2] + ground_forces[2] + winch_forces[2]
        );

    moments = V3d<float>(
        aero_moments[0] + ground_moments[0] + winch_moments[0],
        aero_moments[1] + ground_moments[1] + winch_moments[1],
        aero_moments[2] + ground_moments[2] + winch_moments[2]
        );

    return;
}





// Calculate complete state derivative for integration
// Args:
//     state: Current state [X,Y,Z,u,v,w,qw,qx,qy,qz,p,q,r]
//     forces_body: Aerodynamic forces [Fx, Fy, Fz] (N)
//     moments_body: Aerodynamic moments [L, M, N] (N·m)
//     mass: Aircraft mass (kg)
//     inertia: Inertia tensor [Ixx, Iyy, Izz, Ixz]
// Returns:
//     state_dot: Time derivative of state vector
StateVector<float> Simulation::state_derivative(const StateVector<float>& state, const V3d<float>& forces_body, const V3d<float>& moments_body) {
    // Position derivative
    auto pos_dot = calculate_position_derivative(state);

    // Velocity derivative
    auto vel_dot = calculate_linear_acceleration(state, forces_body);

    // Quaternion derivative
    auto omega = state.angular_velocity();
    auto quat = state.orientation();
    auto quat_dot = quat.derivative(omega);

    // Angular velocity derivative
    auto omega_dot = calculate_angular_acceleration(state, moments_body);

    // Assemble complete derivative
    StateVector<float> state_dot;
    state_dot.set_position(pos_dot);           // [Ẋ, Ẏ, Ż]
    state_dot.set_velocity(vel_dot);           // [u̇, v̇, ẇ]
    state_dot.set_orientation(quat_dot);          // [q̇w, q̇x, q̇y, q̇z]
    state_dot.set_angular_velocity(omega_dot);           // [ṗ, q̇, ṙ]

    return state_dot;
}

//Linear Acceleration (Body Frame)
// Calculate linear acceleration in body frame
// Args:
//     state: Current state vector
//     forces_body: Aerodynamic forces [Fx, Fy, Fz] in body frame (N)
//     mass: Aircraft mass (kg)
// Returns:
//     [u̇, v̇, ẇ] - acceleration in body frame (m/s²)
V3d<float> Simulation::calculate_linear_acceleration(const StateVector<float>& state, const V3d<float>& forces_body) {
    auto velocity = state.velocity();
    auto u = velocity[0];
    auto v = velocity[1];
    auto w = velocity[2];

    auto orientation = state.orientation();

    // roll, pitch & yaw rates
    auto av = state.angular_velocity();
    auto p = av[0];
    auto q = av[1];
    auto r = av[2];

    // Gravity in body frame
    auto g = 9.81f;
    auto g_earth = V3d<float>(0, 0, g);
    auto g_body = orientation.rotate_vector_inverse(g_earth);

    auto mass = aircraft.mass;

    // Total force in body frame
    auto Fx_total = forces_body[0] + mass * g_body[0];
    auto Fy_total = forces_body[1] + mass * g_body[1];
    auto Fz_total = forces_body[2] + mass * g_body[2];

    // Acceleration (including Coriolis terms)
    auto u_dot = Fx_total / mass + r * v - q * w;
    auto v_dot = Fy_total / mass + p * w - r * u;
    auto w_dot = Fz_total / mass + q * u - p * v;


    return V3d<float>(u_dot, v_dot, w_dot);

}

// Angular Acceleration (Body Frame)
//    Calculate angular acceleration in body frame
// Args:
//     state: Current state vector
//     moments_body: Aerodynamic moments [L, M, N] in body frame (N·m)
//     inertia: Inertia tensor components [Ixx, Iyy, Izz, Ixz]
// Returns:
//     [ṗ, q̇, ṙ] - angular acceleration (rad/s²)
V3d<float> Simulation::calculate_angular_acceleration(const StateVector<float>& state, const V3d<float>& moments_body) {
    auto av = state.angular_velocity();
    auto p = av[0];
    auto q = av[1];
    auto r = av[2];

    // Inertia components
    auto Ixx = aircraft.Ixx;
    auto Iyy = aircraft.Iyy;
    auto Izz = aircraft.Izz;
    auto Ixz = aircraft.Ixz;

    auto L = moments_body[0]; // roll moment, 
    auto M = moments_body[1]; // pitch moment,
    auto N = moments_body[2]; // yaw moment

    // Inertia determinant
    auto I_det = Ixx * Izz - Ixz * Ixz;         // note - in practice this is fixed

    // Gyroscopic terms
    auto gyro_L = (Izz - Iyy) * q * r - Ixz * p * q;
    auto gyro_M = (Ixx - Izz) * p * r + Ixz * (p * p - r * r);
    auto gyro_N = (Iyy - Ixx) * p * q + Ixz * q * r;

    // Angular accelerations (Euler's equations)
    auto p_dot = (Izz * (L + gyro_L) + Ixz * (N + gyro_N)) / I_det;
    auto q_dot = (M + gyro_M) / Iyy;
    auto r_dot = (Ixz * (L + gyro_L) + Ixx * (N + gyro_N)) / I_det;

    return V3d<float>(p_dot, q_dot, r_dot);
}



// Position Update (Earth Frame)
//     Calculate position derivative in Earth frame - 
// note, by definition, this is velocity in Earth frame.
// Args:
//     state: Current state vector
// Returns:
//     [Ẋ, Ẏ, Ż] - velocity in Earth frame (m/s)
V3d<float> Simulation::calculate_position_derivative(const StateVector<float>& state) {
    auto v_body = state.velocity();
    auto q = state.orientation();

    // Rotate body velocity to Earth frame
    auto v_earth = q.rotate_vector(v_body);

    return v_earth;
}


// Calculate relative airflow vector in body frame allowing for wind
// Args:
//     state: Current state vector
//     wind_earth: Wind velocity in Earth frame (m/s)
// Returns:
//    new state object with corrected
//    airspeed vector in body frame (m/s)
V3d<float> Simulation::apply_wind_to_state(const StateVector<float>& state, const V3d<float>& wind_earth) {
    auto velocity = state.velocity();
    auto u = velocity[0];
    auto v = velocity[1];
    auto w = velocity[2];

    auto orientation = state.orientation();

    // Rotate wind to body frame
    auto wind_body = orientation.rotate_vector_inverse(wind_earth);

    // Airspeed = ground speed - wind
    auto u_air = u - wind_body[0];
    auto v_air = v - wind_body[1];
    auto w_air = w - wind_body[2];

    return V3d<float>(u_air, v_air, w_air);
}

//     Set up for a winch launch.
// Args:
//     winch_distance: Distance to winch from starting position (m)
//     max_tension: Maximum cable tension (N)
//     weak_link: Weak link breaking tension (N)
void Simulation::setup_winch_launch(float winch_distance, float max_tension, float weak_link) {
    // Position winch ahead of glider (in X direction)
    auto pos = state.position();

    auto winch_pos = V3d<float>(pos[0] + winch_distance, pos[1], 0.0f);  // On ground

    winch = Winch(winch_pos, max_tension, weak_link, winch_distance + 200.0f);
}

//Engage the winch cable.
void Simulation::engage_winch() {
    winch.engage();
}

//Release the winch cable."""
void Simulation::release_winch() {
    winch.release("manual");
}