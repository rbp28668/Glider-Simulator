from math import sqrt, atan2, asin, isnan, isinf

from ask21 import ASK21
from world import World
from state_vector import StateVector
from quaternion import  quaternion_normalize, quaternion_rotate_vector, quaternion_rotate_vector_inverse, quaternion_derivative
from control_inputs import ControlInputs
from model import Model
from ground_contact import GroundContact
from winch import Winch
from v3d import V3d

# State limits to prevent numerical divergence
MAX_VELOCITY = 500.0        # m/s - well beyond any realistic flight speed
MAX_ANGULAR_RATE = 20.0     # rad/s - about 1150 deg/s
MAX_POSITION = 1000000.0    # m - 1000 km


def clamp(value: float, min_val: float, max_val: float) -> float:
    """Clamp value to range [min_val, max_val]."""
    return max(min_val, min(max_val, value))


def safe_value(value: float, default: float = 0.0) -> float:
    """Return default if value is NaN or Inf."""
    if isnan(value) or isinf(value):
        return default
    return value


def sanitize_velocity(vx: float, vy: float, vz: float) -> tuple[float, float, float]:
    """Clamp velocity components to safe range."""
    return (
        clamp(safe_value(vx), -MAX_VELOCITY, MAX_VELOCITY),
        clamp(safe_value(vy), -MAX_VELOCITY, MAX_VELOCITY),
        clamp(safe_value(vz), -MAX_VELOCITY, MAX_VELOCITY)
    )


def sanitize_angular_velocity(p: float, q: float, r: float) -> tuple[float, float, float]:
    """Clamp angular velocity components to safe range."""
    return (
        clamp(safe_value(p), -MAX_ANGULAR_RATE, MAX_ANGULAR_RATE),
        clamp(safe_value(q), -MAX_ANGULAR_RATE, MAX_ANGULAR_RATE),
        clamp(safe_value(r), -MAX_ANGULAR_RATE, MAX_ANGULAR_RATE)
    )


class Simulation:
    """
    Main simulation class that integrates all components and runs the simulation loop.
    """
    def __init__(self):
        self.time_step = 0.01  # seconds
        self.total_time = 0.0  # seconds
        self.world = World()
        self.aircraft = ASK21()
        self.state = StateVector()
        self.controls = ControlInputs()
        self.model = Model()
        self.ground_contact = GroundContact(self.aircraft.cg)
        self.winch = Winch()
        self.winch_info = {}  # Diagnostic info from last winch calculation
        self.pitch_only = False
        self.use_rk4 = True


    def reset(self):
        self.total_time = 0.0
        self.state = StateVector()

    def update(self, dt : float) -> StateVector:
        self.total_time += dt
        print(f"DT: {dt}")
       
        if self.use_rk4:
            return self.update_rk4(dt)
        else:
            return self.update_simple(dt)


    def update_simple(self, dt : float) -> StateVector:
        state = self.state
        # Get wind
        wind_earth = self.world.get_wind_vector(state.position(), self.total_time)

        # Calculate airspeed with wind
        V_air_body = self.apply_wind_to_state(state, wind_earth)

        # Calculate aerodynamics
        forces_body, moments_body = self.model.calculate_aerodynamics(
            state, self.aircraft, self.controls, self.world, V_air_body
        )

        # Get current velocity and angular velocity for Coriolis terms
        u, v, w = state.velocity()
        p, q, r = state.angular_velocity()

        # Linear acceleration: F/m + gravity + Coriolis terms
        # In rotating body frame: a_body = F/m + g_body + ω × v_body
        # Coriolis: (r*v - q*w, p*w - r*u, q*u - p*v)
        acc_x = forces_body[0] / self.aircraft.mass + (r*v - q*w)
        acc_y = forces_body[1] / self.aircraft.mass + (p*w - r*u)
        acc_z = forces_body[2] / self.aircraft.mass + (q*u - p*v)

        orientation = state.orientation()

        # Gravity in body frame
        g = 9.81
        g_earth = (0, 0, g) # +ve down
        g_body = quaternion_rotate_vector_inverse(orientation, g_earth)

        acc_x += g_body[0]
        acc_y += g_body[1]
        acc_z += g_body[2]

        # Update velocity: v_new = v_old + a * dt
        vx = u + acc_x * dt
        vy = v + acc_y * dt
        vz = w + acc_z * dt

        # Update position using OLD velocity (standard forward Euler)
        v_world = quaternion_rotate_vector(orientation, (u, v, w))  # body to world frame
        position = state.position()
        px = position[0] + v_world[0] * dt
        py = position[1] + v_world[1] * dt
        pz = position[2] + v_world[2] * dt

        # Moments of inertia
        Ixx = self.aircraft.Ixx
        Iyy = self.aircraft.Iyy
        Izz = self.aircraft.Izz

        # Angular acceleration with gyroscopic coupling
        # Using Euler's equations: I * ω̇ = M - ω × (I * ω)
        dp = (moments_body[0] + (Iyy - Izz) * q * r) / Ixx
        dq = (moments_body[1] + (Izz - Ixx) * r * p) / Iyy
        dr = (moments_body[2] + (Ixx - Iyy) * p * q) / Izz

        # Update angular velocity
        av_roll = p + dp * dt
        av_pitch = q + dq * dt
        av_yaw = r + dr * dt

        # Note: Artificial damping removed - natural aerodynamic damping comes from:
        # - Tail seeing different AoA due to pitch rate (modeled in tailplane_forces)
        # - For proper damping, add Cmq derivative term to pitching moment

        # Constrain to pitch-only motion if set
        if(self.pitch_only):
            av_yaw = 0
            av_roll = 0
            vy = 0

        # Orientation update via quaternion derivative (using NEW angular velocity)
        quat = state.orientation()
        quat_dot = quaternion_derivative(quat, (av_roll, av_pitch, av_yaw))
        qw = quat[0] + quat_dot[0] * dt
        qx = quat[1] + quat_dot[1] * dt
        qy = quat[2] + quat_dot[2] * dt
        qz = quat[3] + quat_dot[3] * dt

        # Update position using euler angles for DEBUG
        # Note - provides same behaviour
        # roll, pitch, yaw = quaternion_to_euler(quat[0],quat[1], quat[2], quat[3])
        # pitch += angular_velocity[1] * dt
        # quat = euler_to_quaternion(roll, pitch, yaw)
        # qw = quat[0] 
        # qx = quat[1] 
        # qy = quat[2] 
        # qz = quat[3] 

        new_state = StateVector()

        # Sanitize position (allow large range but catch NaN/Inf)
        new_state.set_position((
            clamp(safe_value(px), -MAX_POSITION, MAX_POSITION),
            clamp(safe_value(py), -MAX_POSITION, MAX_POSITION),
            clamp(safe_value(pz), -MAX_POSITION, MAX_POSITION)
        ))

        # Sanitize velocity
        new_state.set_velocity(sanitize_velocity(vx, vy, vz))

        # Sanitize angular velocity
        new_state.set_angular_velocity(sanitize_angular_velocity(av_roll, av_pitch, av_yaw))

        # Sanitize quaternion (normalize handles most issues, but check for NaN)
        quat_safe = (
            safe_value(qw, 1.0),
            safe_value(qx, 0.0),
            safe_value(qy, 0.0),
            safe_value(qz, 0.0)
        )
        new_state.set_orientation(quaternion_normalize(quat_safe))
        new_state.set_forces_moments(forces_body, moments_body)

        self.state = new_state

        return new_state



    def update_rk4(self, dt : float) -> StateVector:

        # Update aircraft state based on physics, control inputs, and world conditions

        """
        Advance simulation by one time step
        """
        # Integration
        self.state = self.rk4_step(
            self.state,
            self.time_step,
            self.calculate_forces_moments,
            self.aircraft.mass,
            [ self.aircraft.Ixx, self.aircraft.Iyy, self.aircraft.Izz, self.aircraft.Ixz ]
        )

        return self.state
    

    def calculate_forces_moments(self, state: StateVector) -> tuple[V3d, V3d]:
        """
        Calculate total forces and moments (aerodynamic + ground contact + winch)

        Args:
            state: Current state vector

        Returns:
            forces_body: [Fx, Fy, Fz] (N)
            moments_body: [L, M, N] (N·m)
        """
        # Get wind
        wind_earth = self.world.get_wind_vector(state.position(), self.total_time)

        # Calculate airspeed with wind
        V_air_body = self.apply_wind_to_state(state, wind_earth)

        # Calculate aerodynamics
        aero_forces, aero_moments = self.model.calculate_aerodynamics(
            state, self.aircraft, self.controls, self.world, V_air_body
        )

        # Calculate ground contact forces
        ground_forces, ground_moments, _ = self.ground_contact.calculate_ground_forces(
            state, self.aircraft.contact_points, self.world, self.aircraft.cg
        )

        # Calculate winch forces
        hook_body = self.aircraft.winch_hook.position_body()
        winch_forces, winch_moments, self.winch_info = self.winch.calculate_forces(
            state, hook_body, self.time_step
        )

        # Accumulate total forces and moments
        forces_body = (
            aero_forces[0] + ground_forces[0] + winch_forces[0],
            aero_forces[1] + ground_forces[1] + winch_forces[1],
            aero_forces[2] + ground_forces[2] + winch_forces[2]
        )

        moments_body = (
            aero_moments[0] + ground_moments[0] + winch_moments[0],
            aero_moments[1] + ground_moments[1] + winch_moments[1],
            aero_moments[2] + ground_moments[2] + winch_moments[2]
        )


        return forces_body, moments_body

 



    def rk4_step(self, state: StateVector, dt:float , forces_moments_func, mass:float, inertia: list[float]) -> StateVector:
        """
        4th order Runge-Kutta integration step
        
        Args:
            state: Current state vector
            dt: Time step (s)
            forces_moments_func: Function that returns (forces, moments)
            mass: Aircraft mass (kg)
            inertia: Inertia tensor
        
        Returns:
            new_state: Updated state vector
        """
        # k1

        forces1, moments1 = forces_moments_func(state)
        k1 = self.state_derivative(state, forces1, moments1, mass, inertia)
        
        # k2
        state2 = state.offset(k1, 0.5*dt) #[state[i] + 0.5*dt*k1[i] for i in range(13)]
        state2.normalize_orientation()
        forces2, moments2 = forces_moments_func(state2)
        k2 = self.state_derivative(state2, forces2, moments2, mass, inertia)
        
        # k3
        state3 = state.offset(k2, 0.5*dt) #[state[i] + 0.5*dt*k2[i] for i in range(13)]
        state3.normalize_orientation()
        forces3, moments3 = forces_moments_func(state3)
        k3 = self.state_derivative(state3, forces3, moments3, mass, inertia)
        
        # k4
        state4 = state.offset(k3, dt) #[state[i] + dt*k3[i] for i in range(13)]
        state4.normalize_orientation()
        forces4, moments4 = forces_moments_func(state4)
        k4 = self.state_derivative(state4, forces4, moments4, mass, inertia)
        
        # Combine & normalize
        new_state = state.rk4_sum(k1, k2, k3, k4, dt)
        new_state.normalize_orientation()
        
        # Set final forces and moments for logging / display
        new_state.set_forces_moments(forces4, moments4)

        return new_state

    @staticmethod
    def state_derivative(state: StateVector, forces_body: V3d, moments_body: V3d, mass: float, inertia)-> StateVector:
        """
        Calculate complete state derivative for integration
        
        Args:
            state: Current state [X,Y,Z,u,v,w,qw,qx,qy,qz,p,q,r]
            forces_body: Aerodynamic forces [Fx, Fy, Fz] (N)
            moments_body: Aerodynamic moments [L, M, N] (N·m)
            mass: Aircraft mass (kg)
            inertia: Inertia tensor [Ixx, Iyy, Izz, Ixz]
        
            
        Returns:
            state_dot: Time derivative of state vector
        """
        # Position derivative
        pos_dot = Simulation.calculate_position_derivative(state)
        
        # Velocity derivative
        vel_dot = Simulation.calculate_linear_acceleration(state, forces_body, mass)
        
        # Quaternion derivative
        omega = state.angular_velocity()
        quat = state.orientation()
        quat_dot = quaternion_derivative(quat, omega)
        
        # Angular velocity derivative
        omega_dot = Simulation.calculate_angular_acceleration(state, moments_body, inertia)
        
        # Assemble complete derivative
        state_dot = StateVector()
        state_dot.set_position(pos_dot)           # [Ẋ, Ẏ, Ż]
        state_dot.set_velocity( vel_dot)           # [u̇, v̇, ẇ]
        state_dot.set_orientation(quat_dot)          # [q̇w, q̇x, q̇y, q̇z]
        state_dot.set_angular_velocity(omega_dot)           # [ṗ, q̇, ṙ]
        
        
        return state_dot


    #Linear Acceleration (Body Frame)
    @staticmethod
    def calculate_linear_acceleration(state: StateVector, forces_body: V3d, mass: float) -> V3d:
        """
        Calculate linear acceleration in body frame
        
        Args:
            state: Current state vector
            forces_body: Aerodynamic forces [Fx, Fy, Fz] in body frame (N)
            mass: Aircraft mass (kg)
        
        Returns:
            [u̇, v̇, ẇ] - acceleration in body frame (m/s²)
        """
        u, v, w = state.velocity()
        orientation = state.orientation()
        p, q, r = state.angular_velocity()
        
        # Gravity in body frame
        g = 9.81
        g_earth = (0, 0, g)
        g_body = quaternion_rotate_vector_inverse(orientation, g_earth)
        
        # Total force in body frame
        Fx_total = forces_body[0] + mass * g_body[0]
        Fy_total = forces_body[1] + mass * g_body[1]
        Fz_total = forces_body[2] + mass * g_body[2]
        
        # Acceleration (including Coriolis terms)
        u_dot = Fx_total/mass + r*v - q*w
        v_dot = Fy_total/mass + p*w - r*u
        w_dot = Fz_total/mass + q*u - p*v
        
        return u_dot, v_dot, w_dot

    # Angular Acceleration (Body Frame)
    @staticmethod
    def calculate_angular_acceleration(state: StateVector, moments_body: V3d , inertia) -> V3d:
        """
        Calculate angular acceleration in body frame
        
        Args:
            state: Current state vector
            moments_body: Aerodynamic moments [L, M, N] in body frame (N·m)
            inertia: Inertia tensor components [Ixx, Iyy, Izz, Ixz]
        
        Returns:
            [ṗ, q̇, ṙ] - angular acceleration (rad/s²)
        """
        p, q, r = state.angular_velocity() # roll pitch & yaw rates
        Ixx, Iyy, Izz, Ixz = inertia
        L, M, N = moments_body             # roll moment, pitch moment, yaw moment
        
        # Inertia determinant
        I_det = Ixx * Izz - Ixz**2         # note - in practice this is fixed
        
        # Gyroscopic terms
        gyro_L = (Izz - Iyy) * q * r - Ixz * p * q
        gyro_M = (Ixx - Izz) * p * r + Ixz * (p**2 - r**2)
        gyro_N = (Iyy - Ixx) * p * q + Ixz * q * r
    
        # Angular accelerations (Euler's equations)
        p_dot = (Izz * (L + gyro_L) + Ixz * (N + gyro_N)) / I_det
        q_dot = (M + gyro_M) / Iyy
        r_dot = (Ixz * (L + gyro_L) + Ixx * (N + gyro_N)) / I_det

        return (p_dot, q_dot, r_dot)


    # Position Update (Earth Frame)
    @staticmethod
    def calculate_position_derivative(state: StateVector) -> V3d:
        """
        Calculate position derivative in Earth frame - 
        note, by definition, this is velocity in Earth frame.
        
        Args:
            state: Current state vector
        
        Returns:
            [Ẋ, Ẏ, Ż] - velocity in Earth frame (m/s)
        """
        v_body = state.velocity()
        q = state.orientation()
        
        # Rotate body velocity to Earth frame
        v_earth = quaternion_rotate_vector(q, v_body)
        
        return v_earth



    @staticmethod
    def apply_wind_to_state(state : StateVector, wind_earth : V3d) -> V3d:
        """
        Calculate relative airflow vector in body frame allowing for wind

        Args:
            state: Current state vector
            wind_earth: Wind velocity in Earth frame (m/s)
        
        Returns:
           new state object with corrected
           airspeed vector in body frame (m/s)
        """
        u, v, w = state.velocity()
        orientation = state.orientation()
        
        # Rotate wind to body frame
        wind_body = quaternion_rotate_vector_inverse(orientation, wind_earth)
        
        # Airspeed = ground speed - wind
        u_air = u - wind_body[0]
        v_air = v - wind_body[1]
        w_air = w - wind_body[2]
        
        return (u_air, v_air, w_air) 

    def setup_winch_launch(self, winch_distance: float = 1000.0,
                           max_tension: float = 6000.0,
                           weak_link: float = 8000.0):
        """
        Set up for a winch launch.

        Args:
            winch_distance: Distance to winch from starting position (m)
            max_tension: Maximum cable tension (N)
            weak_link: Weak link breaking tension (N)
        """
        # Position winch ahead of glider (in X direction)
        pos = self.state.position()
        winch_pos = (pos[0] + winch_distance, pos[1], 0.0)  # On ground

        self.winch = Winch(
            winch_position=winch_pos,
            max_tension=max_tension,
            weak_link=weak_link,
            cable_length=winch_distance + 200.0
        )

    def engage_winch(self):
        """Engage the winch cable."""
        self.winch.engage()

    def release_winch(self):
        """Release the winch cable."""
        self.winch.release("manual")