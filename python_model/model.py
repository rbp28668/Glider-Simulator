
from state_vector import StateVector
from ask21 import ASK21
from control_inputs import ControlInputs
from world import World
from v3d import V3d


from math import sin, cos, tan, asin, atan2, copysign, pi


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
     
        forces_body = (0.0, 0.0, 0.0)
        moments_body = (0.0, 0.0, 0.0)

        alpha = state.AngleOfAttack()
        beta = state.SideslipAngle()
        tas = state.TotalAirspeed()

        #Wings
        for panel in aircraft.wing:
            
            aoa = alpha + cos(beta) * panel.incidence + sin(beta) * aircraft.dihedral_angle  # Simplified effect of sideslip on angle of attack

            ## TODO:  calculate change in aoa due to yaw rate (r) and roll rate (p)

            rootFraction = 1.0 - panel.interp
            tipFraction = panel.interp

            root = panel.rootFoil
            tip = panel.tipFoil if panel.tipFoil is not None else panel.rootFoil

            Clr, Cdr, Cmr = root.coefficients_at(aoa)
            Clt, Cdt, Cmt = tip.coefficients_at(aoa)
            Cl = rootFraction * Clr + tipFraction * Clt
            Cd = rootFraction * Cdr + tipFraction * Cdt 
            Cm = rootFraction * Cmr + tipFraction * Cmt

            q = 0.5 * world.air_density * tas**2
            L = Cl * q * panel.area
            D = Cd * q * panel.area
            M = Cm * q * panel.area * panel.quater_chord

            # TODO - convert L, D to body axes and sum

            

        #Tailplane

        #Fin

        return forces_body, moments_body