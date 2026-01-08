
from state_vector import StateVector
from ask21 import ASK21
from control_inputs import ControlInputs
from world import World
from v3d import V3d, TotalAirspeed, AngleOfAttack, SideslipAngle


from math import degrees, radians, sin, cos, tan, asin, atan2, copysign, pi


class Model :
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
        fin_L, fin_D, fin_M = self.fin_forces(state, aircraft, relative_velocity, control_inputs, world)
        forces_body[0] += fin_D     # drag in body X
        forces_body[1] -= fin_L     # side force in body Y

        # Moments (about c.g.)
        dist = aircraft.fin_quarter_chord - aircraft.cg
        moments_body[2] += fin_L * dist  # yaw moment due to side force at fin quarter chord


        # TODO - some approximation of fuselage drag

        # TODO - Cm_beta : pitch down with sideslip


        return (forces_body[0], forces_body[1], forces_body[2]), (moments_body[0], moments_body[1], moments_body[2])


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
        

        aoa = AngleOfAttack(tailplane_velocity) + aircraft.tailplane_incidence 
        aoa -= controls.pitch * radians(5) # TODO properly - elevator effect, just change AoA up to 5 degrees

        Cl, Cd, Cm = aircraft.tailplane.coefficients_at(aoa)

        tas = TotalAirspeed(tailplane_velocity)
        q = 0.5 * world.air_density * tas * tas
        tp_L = Cl * q * aircraft.tailplane_area
        tp_D = Cd * q * aircraft.tailplane_area
        M = 0 # tp_Cm * tp_q * aircraft.tailplane_area * aircraft.tailplane_quarter_chord  TODO - CM
      

        # convert L, D to body axes and sum
        # Transform to body axes
        D = -tp_D * cos(aoa) - tp_L * sin(aoa)  # drag backwards (hence -ve)
        L = -tp_D * sin(aoa) - tp_L * cos(aoa)  # lift up but +ve z downwards

        #print(f"Tailplane AoA: {degrees(aoa)},  L,D: {L},{D}, Dist:{dist}, Pitch rate: {pitch_rate}, vz_pitch:{vz_pitch},  V_tp: {tailplane_velocity[0]},{tailplane_velocity[1]},{tailplane_velocity[2]}")
     
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
    def add(acc: list[float], v1: V3d, v2: V3d) -> None:
        acc[0] += (v1[0] + v2[0])     # drag in body X
        acc[1] += (v1[1] + v2[1])    # side force in body Y
        acc[2] += (v1[2] + v2[2])    # lift in body z

    
