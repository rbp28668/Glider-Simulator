from math import sqrt, atan2, asin, isnan, isinf

from ask21 import ASK21
from ground_contact import SNAP_TO_ZERO_THRESHOLD
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
  

    def reset(self):
        self.total_time = 0.0
        self.state = StateVector()

    def update(self, dt : float) -> StateVector:
        self.total_time += dt
  
        # Update aircraft state based on physics, control inputs, and world conditions

        """
        Advance simulation by one time step
        """
        # Integration
        self.state = self.rk4_step(
            self.state,
            dt,
            self.calculate_forces_moments,
            self.aircraft.mass,
            [ self.aircraft.Ixx, self.aircraft.Iyy, self.aircraft.Izz, self.aircraft.Ixz ]
        )

        # Apply ground settling - zero velocities if nearly stationary on ground
        self._apply_ground_settling()

        return self.state

    def _apply_ground_settling(self):
        """
        Zero velocities if aircraft is nearly stationary with ground contact.
        This prevents small oscillations when the aircraft should be at rest.
        """
        # Check ground contact
        _, _, contacts = self.ground_contact.calculate_ground_forces(
            self.state, self.aircraft.contact_points, self.world, self.aircraft.cg
        )
        n_contacts = sum(1 for c in contacts if c.in_contact)

        if n_contacts < 2:
            return  # Not enough ground contact

        # Check velocity magnitudes
        u, v, w = self.state.velocity()
        p, q, r = self.state.angular_velocity()
        vel_mag = sqrt(u*u + v*v + w*w)
        ang_mag = sqrt(p*p + q*q + r*r)

        # If below threshold, zero out velocities
        if vel_mag < SNAP_TO_ZERO_THRESHOLD and ang_mag < SNAP_TO_ZERO_THRESHOLD:
            self.state.set_velocity((0.0, 0.0, 0.0))
            self.state.set_angular_velocity((0.0, 0.0, 0.0))


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
        #state2.sanitize()
        forces2, moments2 = forces_moments_func(state2)
        k2 = self.state_derivative(state2, forces2, moments2, mass, inertia)

        # k3
        state3 = state.offset(k2, 0.5*dt) #[state[i] + 0.5*dt*k2[i] for i in range(13)]
        state3.normalize_orientation()
        #state3.sanitize()
        forces3, moments3 = forces_moments_func(state3)
        k3 = self.state_derivative(state3, forces3, moments3, mass, inertia)

        # k4
        state4 = state.offset(k3, dt) #[state[i] + dt*k3[i] for i in range(13)]
        state4.normalize_orientation()
        #state4.sanitize()
        forces4, moments4 = forces_moments_func(state4)
        k4 = self.state_derivative(state4, forces4, moments4, mass, inertia)

        # Combine & normalize
        new_state = state.rk4_sum(k1, k2, k3, k4, dt)
        new_state.normalize_orientation()
        new_state.sanitize()
        
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
    # Angular acceleration is 1st derivative of angular velocity.
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

        #return (p_dot, q_dot, r_dot)

        # We calculate the "Inertial Terms" (Gyroscopic effects) first.
        # These are the terms typically on the RHS of the equations.
        
        # -----------------------------------------------------------------
        # Derived from M = I * w_dot + w x (I * w)
        #

        # Precompute the Determinant (Gamma)
        # This represents the inertial coupling magnitude
        Gamma = Ixx * Izz - Ixz**2
   
        # Pitch (Decoupled in this simplified symmetry):
        # Iyy * dq = M - (Ixx - Izz)*p*r - Ixz*(p^2 - r^2)
        # Note: (Izz - Ixx)*p*r is equivalent to -(Ixx - Izz)*p*r
        
        term_pitch = (Izz - Ixx) * p * r + Ixz * (r**2 - p**2)
        dq = (M + term_pitch) / Iyy

        # Roll and Yaw (Coupled System):
        # We define "Prime" moments (External + Gyroscopic terms)
        # L_prime = L - [ (Izz - Iyy)qr - Ixz*pq ]  <-- from w x (Iw) expansion
        # N_prime = N - [ (Iyy - Ixx)pq + Ixz*qr ]
        
        # CAUTION: Signs often flip depending on moving terms to LHS or RHS.
        # Below is derived for LHS = I*w_dot
        
        L_prime = L + (Iyy - Izz) * q * r + Ixz * p * q
        N_prime = N + (Ixx - Iyy) * p * q - Ixz * q * r

        # Solve using Cramer's Rule (Pre-calculated Gamma)
        dp = (Izz * L_prime + Ixz * N_prime) / Gamma
        dr = (Ixz * L_prime + Ixx * N_prime) / Gamma
        return (dp, dq, dr)
    
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