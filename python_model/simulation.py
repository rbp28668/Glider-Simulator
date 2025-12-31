from math import sqrt, atan2, asin

from ask21 import ASK21
from world import World
from state_vector import StateVector
from quaternion import  quaternion_normalize, quaternion_rotate_vector, quaternion_rotate_vector_inverse, quaternion_derivative
from control_inputs import ControlInputs
from model import Model
from v3d import V3d

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

    def reset(self):
        self.total_time = 0.0
        self.state = StateVector()

    def update(self):
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

        # Update time    
        self.total_time += self.time_step

        return self.state
    

    def calculate_forces_moments(self, state: StateVector) -> tuple[V3d, V3d]:
        """
        Calculate aerodynamic forces and moments
        
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
        forces_body, moments_body = self.model.calculate_aerodynamics(
            state, self.aircraft, self.controls, self.world, V_air_body
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
        p, q, r = state.angular_velocity()
        Ixx, Iyy, Izz, Ixz = inertia
        L, M, N = moments_body
        
        # Inertia determinant
        I_det = Ixx * Izz - Ixz**2
        
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
        u, v, w = state.position()
        orientation = state.orientation()
        
        # Rotate wind to body frame
        wind_body = quaternion_rotate_vector_inverse(orientation, wind_earth)
        
        # Airspeed = ground speed - wind
        u_air = u - wind_body[0]
        v_air = v - wind_body[1]
        w_air = w - wind_body[2]
        
        return (u_air, v_air, w_air) 

