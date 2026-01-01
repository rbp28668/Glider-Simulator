
from state_vector import StateVector
from ask21 import ASK21
from control_inputs import ControlInputs
from world import World
from v3d import V3d, TotalAirspeed, AngleOfAttack, SideslipAngle


from math import radians, sin, cos, tan, asin, atan2, copysign, pi


class Model :
    """
    This is the aerodynamic model for the simulation.
    """
    def __init__(self):
        pass

    
    def calculate_aerodynamics(self, state: StateVector, aircraft: ASK21, control_inputs: ControlInputs, world: World, relative_airflow: V3d) -> tuple[V3d, V3d]:

        """
        Calculate aerodynamic forces and moments
        
        Args:
            state: Current state vector
            aircraft: The aircraft model
            control_inputs: Current control surface deflections
            world: The simulation world
            relative_airflow: The airflow vector relative to the aircraft (in body axes, wind corrected)
        
        Returns:
            forces_body: [Fx, Fy, Fz] (N)
            moments_body: [L, M, N] (N·m)
        """
     
        forces_body = [0.0, 0.0, 0.0]
        moments_body = [0.0, 0.0, 0.0]

   
        #Wings
        for panel in aircraft.wing:

            # Right hand side            
            forces_panel, moments_panel = panel.process(state, relative_airflow, aircraft.cg, world, aircraft.dihedral_angle, control_inputs, 1.0)
            Model.add(forces_body, forces_panel)
            Model.add(moments_body, moments_panel)
            # Left hand side
            forces_panel, moments_panel = panel.process(state, relative_airflow, aircraft.cg, world, aircraft.dihedral_angle, control_inputs, -1.0)
            Model.add(forces_body, forces_panel)
            Model.add(moments_body, moments_panel)

        #Tailplane

        tp_L, tp_D, tp_M = self.tailplane_forces(state, aircraft, relative_airflow, control_inputs, world) 
        forces_body[0] += tp_D     # drag in body X
        forces_body[2] += tp_L     # lift in body z
        # Moments (about c.g.)
        dist = aircraft.tailplane_quarter_chord - aircraft.cg
        moments_body[1] += tp_M + tp_L * dist  # pitch moment due to lift at tailplane quarter chord (-ve lift is tailplene up so nose down)

        #Fin
        fin_L, fin_D, fin_M = self.fin_forces(state, aircraft, relative_airflow, control_inputs, world)
        forces_body[0] += fin_D     # drag in body X
        forces_body[1] -= fin_L     # side force in body Y

        # Moments (about c.g.)
        dist = aircraft.fin_quarter_chord - aircraft.cg
        moments_body[2] += fin_L * dist  # yaw moment due to side force at fin quarter chord

        return (forces_body[0], forces_body[1], forces_body[2]), (moments_body[0], moments_body[1], moments_body[2])

    def tailplane_forces(self, state: StateVector, aircraft: ASK21, relative_airflow: V3d, controls: ControlInputs,  world: World) -> V3d:
        """
        Calculate tailplane aerodynamic forces
        
        Args:
            state: Current state vector
            aircraft: The aircraft model
            relative_airflow: The airflow vector relative to the aircraft (in body axes, wind corrected)
            world: The simulation world
        
        Returns:
            Tuple with (Lift, Drag, Moment) from tailplane
        """
        tailplane_airflow = ( relative_airflow[0], relative_airflow[1], relative_airflow[2] + state.angular_velocity()[1] * aircraft.tailplane_quarter_chord) # TODO sign?
        tp_aoa = AngleOfAttack(tailplane_airflow) + aircraft.tailplane_incidence 
        tp_aoa -= controls.pitch * radians(5) # TODO properly - elevator effect
        
        tp_Cl, tp_Cd, tp_Cm = aircraft.tailplane.coefficients_at(tp_aoa)
        tp_tas = TotalAirspeed(tailplane_airflow)
        tp_q = 0.5 * world.air_density * tp_tas**2
        tp_L = tp_Cl * tp_q * aircraft.tailplane_area
        tp_D = tp_Cd * tp_q * aircraft.tailplane_area
        M = 0 # tp_Cm * tp_q * aircraft.tailplane_area * aircraft.tailplane_quarter_chord  TODO - CM

        # convert L, D to body axes and sum
        # Transform to body axes
        D = -tp_D * cos(tp_aoa) - tp_L * sin(tp_aoa)
        L = -tp_D * sin(tp_aoa) - tp_L * cos(tp_aoa)
        return (L,D, M)
  
    def fin_forces(self, state: StateVector, aircraft: ASK21, relative_airflow: V3d, controls: ControlInputs,  world: World) -> V3d:
        fin_airflow = ( relative_airflow[0], relative_airflow[1] + state.angular_velocity()[2] * aircraft.fin_quarter_chord, relative_airflow[2]) # TODO sign?
        fin_aoa = SideslipAngle(fin_airflow)  
        
        # TODO properly!! - rudder effect
        fin_aoa += controls.rudder * radians(10)  # max 10 degrees deflection
        
        fin_Cl, fin_Cd, fin_Cm = aircraft.fin.coefficients_at(fin_aoa)
        fin_tas = TotalAirspeed(fin_airflow)
        fin_q = 0.5 * world.air_density * fin_tas**2
        fin_L = fin_Cl * fin_q * aircraft.fin_area 
        fin_D = fin_Cd * fin_q * aircraft.fin_area

        D = -fin_D * cos(fin_aoa) - fin_L * sin(fin_aoa)
        L = -fin_D * sin(fin_aoa) - fin_L * cos(fin_aoa)  # lift to right should be +ve
        return (L,D, 0.0)  # No fin moment for now

    @staticmethod
    def add(acc: list[float], v: V3d) -> None:
        acc[0] += v[0]     # drag in body X
        acc[1] += v[1]     # side force in body Y
        acc[2] += v[2]     # lift in body z

    
