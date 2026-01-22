from state_vector import StateVector
from ask21 import ASK21
from control_inputs import ControlInputs
from world import World
from v3d import V3d, TotalAirspeed, AngleOfAttack, SideslipAngle

from math import radians, sin, cos, isnan, isinf

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
        pass

    
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

        #print(f'Moments Body: {moments_body[0]},{moments_body[1]},{moments_body[2]}')

        #Tailplane

        tp_L, tp_D, tp_M = self.tailplane_forces(state, aircraft, relative_velocity, control_inputs, world) 
        forces_body[0] += tp_D     # drag in body X
        forces_body[2] += tp_L     # lift in body z
        # Moments (about c.g.)
        dist = aircraft.tailplane_quarter_chord - aircraft.cg
        moments_body[1] += tp_M - tp_L * dist  # pitch moment due to lift at tailplane quarter chord (- sign as dist is -ve as behind c.g.)
        #print(f'Tailplane Effect -  Moment: {moments_body[1]}, L:{tp_L}, D:{tp_D}, M:{tp_M}')

        #Fin
        fin_L, fin_D, fin_yaw_damping = self.fin_forces(state, aircraft, relative_velocity, control_inputs, world)
        forces_body[0] += fin_D     # drag in body X
        forces_body[1] -= fin_L     # side force in body Y

        # Moments (about c.g.)
        dist = aircraft.fin_quarter_chord - aircraft.cg
        moments_body[2] += fin_L * dist  # yaw moment due to side force at fin quarter chord
        moments_body[2] += fin_yaw_damping  # explicit yaw damping


        # Fuselage drag approximation
        # ASK-21 fuselage equivalent flat plate area ~0.025 m² (typical for training glider)
        # This includes fuselage, canopy, wing-fuselage interference, control surface gaps, etc.
        fuselage_Cd_S = 0.025  # m² equivalent flat plate area
        tas = TotalAirspeed(relative_velocity)
        if tas > MIN_AIRSPEED:
            q = 0.5 * world.air_density * tas * tas
            fuselage_drag = fuselage_Cd_S * q
            forces_body[0] -= fuselage_drag  # drag acts backward (-X direction)

        # TODO - Cm_beta : pitch down with sideslip

        # Sanitize and clamp final forces/moments to prevent numerical overflow
        fx = clamp(safe_value(forces_body[0]), -MAX_FORCE, MAX_FORCE)
        fy = clamp(safe_value(forces_body[1]), -MAX_FORCE, MAX_FORCE)
        fz = clamp(safe_value(forces_body[2]), -MAX_FORCE, MAX_FORCE)
        mx = clamp(safe_value(moments_body[0]), -MAX_MOMENT, MAX_MOMENT)
        my = clamp(safe_value(moments_body[1]), -MAX_MOMENT, MAX_MOMENT)
        mz = clamp(safe_value(moments_body[2]), -MAX_MOMENT, MAX_MOMENT)

        return (fx, fy, fz), (mx, my, mz)


    def tailplane_forces(self, state: StateVector, aircraft: ASK21, relative_velocity: V3d, controls: ControlInputs,  world: World) -> V3d:
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
            return (0.0, 0.0, 0.0)

        aoa = AngleOfAttack(tailplane_velocity) + aircraft.tailplane_incidence
        aoa -= controls.pitch * radians(5)  # TODO properly - elevator effect

        Cl, Cd, Cm = aircraft.tailplane.coefficients_at(aoa)

        q = 0.5 * world.air_density * tas * tas
        tp_L = Cl * q * aircraft.tailplane_area
        tp_D = Cd * q * aircraft.tailplane_area
        M = 0  # TODO - CM

        # Transform from wind axes to body axes (rotation by angle of attack about Y)
        D = -tp_D * cos(aoa) - tp_L * sin(aoa)  # drag backwards
        L =  tp_D * sin(aoa) - tp_L * cos(aoa)  # lift up is -ve Z in body axes

        return (safe_value(L), safe_value(D), safe_value(M))
  
    def fin_forces(self, state: StateVector, aircraft: ASK21, relative_airflow: V3d, controls: ControlInputs,  world: World) -> V3d:
        # Distance from CG to fin (positive = aft of CG)
        fin_arm = aircraft.cg - aircraft.fin_quarter_chord  # positive value (~4.78m)

        # Yaw rate effect on fin airflow
        # When yawing right (r > 0), fin swings left, sees airflow from right (increased v)
        # Fin velocity = ω × r = (0, r * x_fin, 0) where x_fin < 0, so v_fin < 0
        # Relative airflow = aircraft_airflow - fin_velocity, so v increases
        yaw_rate = state.angular_velocity()[2]
        fin_airflow = (relative_airflow[0],
                       relative_airflow[1] - yaw_rate * aircraft.fin_quarter_chord,
                       relative_airflow[2])

        fin_tas = TotalAirspeed(fin_airflow)

        # Low airspeed protection
        if fin_tas < MIN_AIRSPEED:
            return (0.0, 0.0, 0.0)

        fin_aoa = SideslipAngle(fin_airflow)

        # Rudder effect
        fin_aoa += controls.rudder * radians(15)  # max 15 degrees deflection

        fin_Cl, fin_Cd, fin_Cm = aircraft.fin.coefficients_at(fin_aoa)
        fin_q = 0.5 * world.air_density * fin_tas**2
        fin_L = fin_Cl * fin_q * aircraft.fin_area
        fin_D = fin_Cd * fin_q * aircraft.fin_area

        # Additional yaw damping (Cnr effect) - opposes yaw rate
        # This represents damping from fuselage, fin boundary layer, etc.
        # Negative sign ensures moment opposes yaw rate (damping, not divergence)
        Cnr = 0.05  # yaw damping coefficient
        yaw_damping = -Cnr * yaw_rate * fin_q * aircraft.fin_area * fin_arm

        # Transform from wind axes to body axes
        D = -fin_D * cos(fin_aoa) - fin_L * sin(fin_aoa)  # drag backwards
        L =  fin_D * sin(fin_aoa) - fin_L * cos(fin_aoa)  # side force (fin "lift")

        # Return side force, drag, and yaw damping moment
        return (safe_value(L), safe_value(D), safe_value(yaw_damping))

    @staticmethod
    def add(acc: list[float], v1: V3d, v2: V3d) -> None:
        acc[0] += (v1[0] + v2[0])     # drag in body X
        acc[1] += (v1[1] + v2[1])    # side force in body Y
        acc[2] += (v1[2] + v2[2])    # lift in body z

    
