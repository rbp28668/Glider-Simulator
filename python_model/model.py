from state_vector import StateVector
from ask21 import ASK21
from control_inputs import ControlInputs
from world import World
from v3d import V3d, TotalAirspeed, AngleOfAttack, SideslipAngle

from math import radians, sin, cos, isnan, isinf, copysign, atan2, sqrt, asin

# Minimum airspeed for aerodynamic calculations (m/s)
MIN_AIRSPEED = 1.0

# Maximum total force/moment to prevent numerical overflow
MAX_FORCE = 100000.0
MAX_MOMENT = 500000.0


def clamp(value: float, min_val: float, max_val: float) -> float:
    """Clamp value to range [min_val, max_val]."""
    return max(min_val, min(max_val, value))


def safe_value(value: float, default: float = 0.0) -> float:
    """Return default if value is NaN or Inf."""
    if isnan(value) or isinf(value):
        return default
    return value


class Model:
    """
    This is the aerodynamic model for the simulation.
    """
    def __init__(self):
        self.ZERO : tuple[V3d,V3d] = ((0.0, 0.0, 0.0), (0.0,0.0,0.0))

    
    def calculate_aerodynamics(self, state: StateVector, aircraft: ASK21, control_inputs: ControlInputs, world: World, relative_velocity: V3d) -> tuple[V3d, V3d]:

        """
        Calculate aerodynamic forces and moments
        
        Args:
            state: Current state vector
            aircraft: The aircraft model
            control_inputs: Current control surface deflections
            world: The simulation world
            relative_velocity: The aircraft velocity relative to the air-mass (in body axes, wind corrected)
        
        Returns:
            forces_body: [Fx, Fy, Fz] (N)
            moments_body: [L, M, N] (N·m)
        """
     

        forces_body = [0.0, 0.0, 0.0]
        moments_body = [0.0, 0.0, 0.0]

        #print(f'Model Relative Airflow {relative_airflow[0]}, {relative_airflow[1]},{relative_airflow[2]}')

        wing_forces, wing_moments = self.wing_forces_moments(state, aircraft, relative_velocity, control_inputs, world)
        self.accumulate(forces_body, wing_forces)
        self.accumulate(moments_body, wing_moments)

        #Tailplane
        tailplane_forces, tailplane_moments = self.tailplane_forces_moments(state, aircraft, relative_velocity, control_inputs, world) 
        self.accumulate(forces_body, tailplane_forces)
        self.accumulate(moments_body, tailplane_moments)
        
        #Fin
        fin_forces, fin_moments = self.fin_forces_moments(state, aircraft, relative_velocity, control_inputs, world)
        self.accumulate(forces_body, fin_forces)
        self.accumulate(moments_body, fin_moments)


        # Fuselage
        fuselage_forces, fuselage_moments =  self.fuselage_forces_moments(state, aircraft, relative_velocity, world)
        #fuselage_forces, fuselage_moments =  self.yet_another_fuselage_model(state, aircraft, relative_velocity, world)
        self.accumulate(forces_body, fuselage_forces)
        self.accumulate(moments_body, fuselage_moments)

        # Explicit roll damping (Clp effect)
        # Wing panels provide some damping via local velocity, but add explicit term
        # for robustness at high rates. Clp is typically -0.4 to -0.5 for gliders.
        # roll_rate = state.angular_velocity()[0]
        # yaw_rate = state.angular_velocity()[2]
        # tas = TotalAirspeed(relative_velocity)
        # q = 0 # dynamic pressure
        # if tas > MIN_AIRSPEED:
        #     q = 0.5 * world.air_density * tas * tas
        #     Clp = -0.4  # roll damping derivative
        #     wing_span = aircraft.wing_span
        #     wing_area = wing_span**2 / aircraft.get_params().AR  # S = b²/AR
        #     # Damping moment: Clp * (p * b/2V) * q * S * b
        #     # Simplified: Clp * p * q * S * b² / (2V)
        #     roll_damping = Clp * roll_rate * q * wing_area * wing_span / (2 * tas)
        #     moments_body[0] += roll_damping

        #     # Adverse yaw from aileron (Cn_δa effect)
        #     # Down-going aileron increases lift and induced drag, up-going decreases it
        #     # This drag differential creates yaw opposite to roll direction
        #     Cn_da = -0.01  # adverse yaw derivative (per unit roll command)
        #     adverse_yaw = Cn_da * control_inputs.roll * q * wing_area * wing_span
        #     moments_body[2] += adverse_yaw


        # TODO - Cm_beta : pitch down with sideslip

        # High-rate damping to prevent unrealistic spin-up during stall
        # This always applies, regardless of airspeed, to ensure stability
        # when normal aero damping breaks down at high AoA
        HIGH_RATE_THRESHOLD = 0.5  # rad/s (~30 deg/s) - above this, extra damping kicks in
        HIGH_RATE_DAMP = 8000.0    # N.m.s/rad - strong damping coefficient

        roll_rate = state.angular_velocity()[0]
        yaw_rate = state.angular_velocity()[2]

        if abs(roll_rate) > HIGH_RATE_THRESHOLD:
            excess_rate = roll_rate - copysign(HIGH_RATE_THRESHOLD, roll_rate)
            moments_body[0] -= HIGH_RATE_DAMP * excess_rate

        if abs(yaw_rate) > HIGH_RATE_THRESHOLD:
            excess_rate = yaw_rate - copysign(HIGH_RATE_THRESHOLD, yaw_rate)
            moments_body[2] -= HIGH_RATE_DAMP * excess_rate

        # Sanitize and clamp final forces/moments to prevent numerical overflow
        fx = clamp(safe_value(forces_body[0]), -MAX_FORCE, MAX_FORCE)
        fy = clamp(safe_value(forces_body[1]), -MAX_FORCE, MAX_FORCE)
        fz = clamp(safe_value(forces_body[2]), -MAX_FORCE, MAX_FORCE)
        mx = clamp(safe_value(moments_body[0]), -MAX_MOMENT, MAX_MOMENT)
        my = clamp(safe_value(moments_body[1]), -MAX_MOMENT, MAX_MOMENT)
        mz = clamp(safe_value(moments_body[2]), -MAX_MOMENT, MAX_MOMENT)

        return (fx, fy, fz), (mx, my, mz)


    # Calculate the forces and moments of the wing.
    def wing_forces_moments(self, state: StateVector, aircraft: ASK21, relative_velocity: V3d, control_inputs: ControlInputs, world: World) -> tuple[V3d, V3d]:
        forces_body = [0.0, 0.0, 0.0]
        moments_body = [0.0, 0.0, 0.0]

        #print(f'Model Relative Airflow {relative_airflow[0]}, {relative_airflow[1]},{relative_airflow[2]}')

        params = aircraft.get_params()
   
        #Wings
        for panel in aircraft.wing:

            # Right hand side            
            r_forces_panel, r_moments_panel = panel.process(state, relative_velocity, params, world, control_inputs, 1.0)
            # Left hand side
            l_forces_panel, l_moments_panel = panel.process(state, relative_velocity, params, world, control_inputs, -1.0)

            #print(f'Moments: {l_moments_panel[0] + r_moments_panel[0]},{l_moments_panel[1] + r_moments_panel[1]},{l_moments_panel[2] + r_moments_panel[2]},')
            Model.add(forces_body, l_forces_panel, r_forces_panel)
            Model.add(moments_body, l_moments_panel, r_moments_panel)

        return (forces_body[0], forces_body[1], forces_body[2]), (moments_body[0], moments_body[1], moments_body[2])
 

    def tailplane_forces_moments(self, state: StateVector, aircraft: ASK21, relative_velocity: V3d, controls: ControlInputs,  world: World) -> tuple[V3d, V3d]:
        """
        Calculate tailplane aerodynamic forces
        
        Args:
            state: Current state vector
            aircraft: The aircraft model
            relative_velocity: The aircraft velocity relative to the air-mass (in body axes, wind corrected)
            world: The simulation world
        
        Returns:
            Tuple with (Lift, Drag, Moment) from tailplane
        """

        #Allow for pitch rate to change airflow at tail.  Pitching up then tail going down (+ve direction)
        dist = aircraft.tailplane_quarter_chord - aircraft.cg  # distance of tailplane A/C from c of g  (-ve as behind)
        pitch_rate = state.angular_velocity()[1]               # +ve pitch rate -> nose up so tail down (+ve z dirn)
        vz_pitch = pitch_rate * -dist

        tailplane_velocity = ( 
            relative_velocity[0],  # u - velocity forward
            relative_velocity[1],  # v - velocity to right
            relative_velocity[2] + vz_pitch) # add in extra vertical velocity do to pitch rate
        

        tas = TotalAirspeed(tailplane_velocity)

        # Low airspeed protection
        if tas < MIN_AIRSPEED:
            return self.ZERO

        aoa_raf = AngleOfAttack(tailplane_velocity)  # local angle of attack at tail from relative airflow velocity
        aoa = aoa_raf + aircraft.tailplane_incidence

        # Elevator effect: positive pitch input (forward stick) reduces tailplane AoA
        # Forward stick → elevator trailing edge DOWN → less lift at tail → nose down
        elevator_deflection = controls.pitch * aircraft.elevator_max_deflection
        aoa -= elevator_deflection * 0.6  # Elevator effectiveness ~0.6 (plain flap factor)

        Cl, Cd, Cm = aircraft.tailplane.coefficients_at(aoa)

        q = 0.5 * world.air_density * tas * tas
        tp_L = Cl * q * aircraft.tailplane_area
        tp_D = Cd * q * aircraft.tailplane_area

        # Pitching moment from tailplane airfoil Cm
        # M = Cm * q * S * c (positive Cm = nose up)
        M = Cm * q * aircraft.tailplane_area * aircraft.tailplane_chord


        # Transform from wind axes to body axes (rotation by angle of attack about Y)
        # Wind axes: -X is drag direction, -Z is lift direction
        # Body axes: X forward, Z down
        D = -(tp_D * cos(aoa_raf) - tp_L * sin(aoa_raf))  # drag backwards
        L = -(tp_D * sin(aoa_raf) + tp_L * cos(aoa_raf))  # lift is -ve Z in body axes

        forces = (D, 0.0, L)

    
        # Moments (about c.g.). Note: dist is negative when tail is behind CG.
        # pitchMoment = M + L * dist where positive L (downwards) and positive dist
        # produce a positive pitchMoment (nose-up) as expected for tail downforce.
        pitchMoment = M + L * -dist

        moments = (0.0, pitchMoment, 0.0)

        return (forces, moments)


    def fin_forces_moments(self, state: StateVector, aircraft: ASK21, relative_airflow: V3d, controls: ControlInputs, world: World) -> tuple[V3d, V3d]:
        # Distance from CG to fin (should be negative if behind CG, e.g., -4.78)
        # Let's ensure we use the actual relative position vector
        dist_behind_cg =  aircraft.cg - aircraft.fin_quarter_chord # +ve 
        assert(dist_behind_cg > 0.0), "Fin should be located behind CG"

        # 1. Local Airflow calculation
        # If r > 0, tail moves left. Relative air comes from the right (+Y).
        yaw_rate = state.angular_velocity()[2]
        
        # relative_airflow is V_air - V_body. 
        # If aircraft moves forward, relative_airflow[0] is negative (headwind).
        # We need the local relative airflow at the fin:
        v_y_induced = yaw_rate * dist_behind_cg
        
        local_airflow = (
            relative_airflow[0],
            relative_airflow[1] - v_y_induced,   # fin moves left from right yaw rate
            relative_airflow[2]
        )

        fin_tas = TotalAirspeed(local_airflow)
        if fin_tas < 1.0:
            return self.ZERO # Return 0 if stationary

        # 2. Beta at the fin
        # If local_airflow[1] is positive (air from right), beta is positive.
        beta_fin_raf = atan2(local_airflow[1], local_airflow[0])

        # Rudder: Right rudder (+1) should pull the tail LEFT (+Fy) to yaw nose RIGHT.
        # This means right rudder must create 'negative lift' in aero terms.
        beta_fin = beta_fin_raf +controls.rudder * radians(15) 

        # 3. Aero Coefficients
        cl, cd, _ = aircraft.fin.coefficients_at(beta_fin)
        q = 0.5 * world.air_density * fin_tas**2
        
        # In a vertical fin, 'Lift' is side force. 
        # If cl is positive (due to +beta), the wing lifts 'left' (-Y).
        lift = cl * q * aircraft.fin_area
        drag = cd * q * aircraft.fin_area

        # 4. Final Body Forces
        # Drag acts in direction of local airflow (mostly -X)
        # Lift acts perpendicular to local airflow (mostly -Y)
        fx = -(drag * cos(beta_fin_raf) - lift * sin(beta_fin_raf))  # drag backwards, lift can have small forward component at high beta
        fy = -(lift * cos(beta_fin_raf) + drag * sin(beta_fin_raf))

        forces = (fx, fy, 0.0)

        # Moments (about c.g.)
        # Yaw moment = position_x × Fy = dist × fin_sideforce
        dist = aircraft.fin_quarter_chord - aircraft.cg     # -ve as fin behind c.g.
        m_yaw = fy * dist  # yaw moment from side force at fin

        moments = (0.0, 0.0, m_yaw)      # Assume no pitch or roll moments from fin
   

        return forces, moments
    


    def fuselage_forces_moments(self, state: StateVector, aircraft: ASK21, relative_airflow: V3d, world: World) -> tuple[V3d, V3d]:
        """
        Calculate forces and moments for the fuselage using a slender-body 
        aerodynamic approximation.
        """
        u, v, w = relative_airflow
        V_sq = u*u + v*v + w*w
        V = sqrt(V_sq)
        
        # Avoid division by zero at rest
        if V < 0.1:
            return self.ZERO

        # 1. Environment and Dynamic Pressure
        rho = world.air_density #(state.position()[2])
        q_inf = 0.5 * rho * V_sq
        
        # 2. Local Flow Angles
        # alpha (pitch) and beta (sideslip)
        alpha = atan2(w, u) if u != 0 else 0.0
        beta = asin(clamp(v / V, -1.0, 1.0))

        # 3. Aerodynamic Coefficients for ASK21 Fuselage
        # These are typical values for a high-performance tandem glider
        C_d0 = aircraft.C_d0       # Baseline parasite drag
        C_y_beta = aircraft.C_y_beta   # Side force coefficient per radian
        C_z_alpha = aircraft.C_z_alpha  # Vertical force coefficient (negligible lift)
        C_m_alpha = aircraft.C_m_alpha   # Pitching instability (destabilizing)
        C_n_beta = aircraft.C_n_beta   # Yawing instability (Munk moment)

        # 4. Force Calculation (Body Frame)
        # Drag is always opposite to the velocity vector
        drag = q_inf * aircraft.S * (C_d0 + 0.1 * alpha**2) # Simplified polar
        
        # Convert drag to body components and add transverse forces
        fx = -drag * (u / V)
        fy = q_inf * aircraft.S * C_y_beta * beta
        fz = q_inf * aircraft.S * (C_z_alpha * alpha) - (drag * (w / V))

        forces_body = (fx, fy, fz)

        # 5. Moment Calculation (Body Frame)
        # Moments are referenced to the Mean Aerodynamic Chord (c_bar) and Wingspan (b)
        l_roll = 0.0 # Fuselage roll contribution is usually negligible
        c_bar = aircraft.mean_chord
        b = aircraft.wing_span
        m_pitch = q_inf * aircraft.S * c_bar * C_m_alpha * alpha
        n_yaw = q_inf * aircraft.S * b * C_n_beta * beta

        # Note - yaw moment is an approximation and is only valid for small beta.
        # Munk moment: Mm = -1/2 . (Azz − Axx). U**2 . sin (2∂)
        # This starts at zero for zero beta and returns to zero at 90 degrees
        # Note also that vortex shedding at higher values of beta is likely to be stablising
        # as well as creating drag.  So, at the moment:
        m_pitch = 0
        n_yaw = 0 

        #

        moments_body = (l_roll, m_pitch, n_yaw)

        return forces_body, moments_body

    # see https://gemini.google.com/share/aaa41155a81e
    def yet_another_fuselage_model(self,state: StateVector, aircraft: ASK21, relative_airflow: V3d, world: World) -> tuple[V3d, V3d]:

        u, v, w = relative_airflow
        V_sq = u*u + v*v + w*w
        V = sqrt(V_sq)
        
        # Avoid division by zero at rest
        if V < 0.1:
            return self.ZERO

        # 1. Environment and Dynamic Pressure
        rho = world.air_density #(state.position()[2])
        q_dyn = 0.5 * rho * V**2
        
        # 2. Local Flow Angles
        # alpha (pitch) and beta (sideslip)
        alpha = atan2(w, u) if u != 0 else 0.0
        beta = asin(clamp(v / V, -1.0, 1.0))

        # Original linear coefficients (still used for small-angle slopes)
        # Gives sideforce for beta and lift for alpha
        Cyb_linear = -0.180
        Cza_linear = 0.080        

        # --- 1. LATERAL FORCE (Sideforce) ---
        # Component of velocity perpendicular to the fuselage side
        # Linear part + Cross-flow part
        # Cy_total = Cy_linear * beta + Cd_cross * sin^2(beta)
        
        # We use sign(beta) to ensure force is always resistive
        cross_flow_y = -aircraft.Cd_cylinder * (aircraft.S_side / aircraft.S) * sin(beta) * abs(sin(beta))
        linear_y = Cyb_linear * sin(beta) # use sin(beta) for better range
        
        # Total Sideforce coefficient (blended naturally by the sin terms)
        Cy = linear_y + cross_flow_y


        # --- 2. NORMAL FORCE (Lift-ish) ---
        cross_flow_z = -aircraft.Cd_cylinder * (aircraft.S_plan / aircraft.S) * sin(alpha) * abs(sin(alpha))
        linear_z = -Cza_linear * sin(alpha)
        
        Cz = linear_z + cross_flow_z

        # --- 3. AXIAL FORCE (Drag) ---
        # At high angles, the 'front' of the fuselage is no longer the only drag source
        # This simplifies to base drag + induced drag effects
        Cx = aircraft.C_d0 * cos(alpha) * cos(beta) - abs(0.5 * Cz * sin(alpha))


        # --- 4. MOMENTS ---
        # Damping still scales linearly but with high-alpha 'blanking' logic
        # If alpha > 25 degrees, damping reduces because airflow is turbulent
        blanking = 1.0 if abs(alpha) < 0.4 else max(0.2, 1.0 - (abs(alpha) - 0.4))
        
        p,q,r = state.angular_velocity()
        q_hat = (q * aircraft.mean_chord) / (2 * V)
        r_hat = (r * aircraft.wing_span) / (2 * V)

        
        Cm = (0.065 * sin(alpha) + -0.120 * q_hat) * blanking
        Cn = (-0.090 * sin(beta) + -0.025 * r_hat) * blanking
   
        forces = (Cx * q_dyn * aircraft.S, Cy * q_dyn * aircraft.S, Cz * q_dyn * aircraft.S)
        moments = (0.0, Cm * q_dyn * aircraft.S * aircraft.mean_chord, Cn * q_dyn * aircraft.S * aircraft.wing_span)

        return forces, moments


        # old Fuselage drag approximation
        # ASK-21 fuselage equivalent flat plate area ~0.025 m² (typical for training glider)
        # This includes fuselage, canopy, wing-fuselage interference, control surface gaps, etc.
        # fuselage_Cd_S = 0.025  # m² equivalent flat plate area
        # if tas > MIN_AIRSPEED:
        #     beta = SideslipAngle(relative_velocity)
        #     sin_beta = sin(beta)
        #     cos_beta = cos(beta)

        #     # Base forward drag (streamlined flight)
        #     fuselage_drag = fuselage_Cd_S * q
        #     forces_body[0] -= fuselage_drag  # drag acts backward (-X direction)

        #     # Additional forward drag due to sideslip (fuselage no longer streamlined)
        #     # Side area ~5 m² contributes to forward drag proportional to sin²(beta)
        #     # This causes rapid descent in a full slip - used by glider pilots to lose altitude
        #     fuselage_side_Cd_S = 5.0  # m² effective side area * Cd
        #     sideslip_drag = fuselage_side_Cd_S * q * sin_beta * sin_beta
        #     forces_body[0] -= sideslip_drag  # additional forward drag in sideslip

        #     # Side force from fuselage (opposes sideslip velocity)
        #     side_force = fuselage_side_Cd_S * q * sin_beta * abs(sin_beta)
        #     forces_body[1] -= side_force  # opposes sideslip


    @staticmethod
    def accumulate(acc: list[float], delta: V3d) -> None:
        acc[0] += delta[0]
        acc[1] += delta[1]
        acc[2] += delta[2]

    @staticmethod
    def add(acc: list[float], v1: V3d, v2: V3d) -> None:
        acc[0] += (v1[0] + v2[0])     # drag in body X
        acc[1] += (v1[1] + v2[1])    # side force in body Y
        acc[2] += (v1[2] + v2[2])    # lift in body z

    
