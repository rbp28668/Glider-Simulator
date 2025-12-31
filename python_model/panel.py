from aerofoil import Aerofoil
from math import atan2, cos, radians, sin

from control_inputs import ControlInputs
from state_vector import StateVector
from v3d import AngleOfAttack, SideslipAngle, TotalAirspeed, V3d
from world import World

class Panel:
    """
    Panel is part of a wing.  It may be extended to include control surfaces such as ailerons or airbrakes.
    """
    def __init__(self, span: float, area: float, quater_chord: float, incidenceDegrees: float, rootFoil : Aerofoil , tipFoil : Aerofoil, interp : float = 0.0):
        self.mid_span = span
        self.area = area
        self.quater_chord = quater_chord
        self.incidence = radians(incidenceDegrees)
        self.rootFoil = rootFoil
        self.tipFoil = tipFoil
        self.interp = interp  # interpolation factor between root and tip aerofoils (0.0 = root, 1.0 = tip)


    def process(self, state: StateVector, relative_airflow: V3d, world: World, dihedral: float, controls: ControlInputs, sign: float) -> tuple[V3d, V3d]:

        # Get local airflow at panel due to angular velocity
        local_airflow = self.local_airflow(state, relative_airflow, sign)

        local_tas = TotalAirspeed(local_airflow)
        local_alpha = AngleOfAttack(local_airflow)
        beta = SideslipAngle(local_airflow)

        # Effective AoA for sideslip 
        aoa_beta = atan2(local_airflow[1], local_airflow[0])

        local_alpha += self.incidence  # add geometric incidence angle
        aoa_beta += dihedral  # add dihedral effect

        aoa = local_alpha * cos(beta) + aoa_beta * sin(beta)

        Cl, Cd, Cm = self.coefficients_at(aoa)

        q = 0.5 * world.air_density * local_tas**2
        L = Cl * q * self.area
        D = Cd * q * self.area
        M = Cm * q * self.area * self.quater_chord

        # convert L, D to body axes and sum
        # Transform to body axes
        Fx = -D * cos(aoa) + L * sin(aoa)
        Fz = -D * sin(aoa) - L * cos(aoa)

        forces_body = Fx, 0.0, Fz  # drag in body X, side force 0, lift in body z
        
        # Moments (about c.g.)
        # Assume wing is centered on fuselage centerline (no spanwise moment)
        # Roll moment due to lift at panel mid-span
        moments_body = Fz * self.mid_span * sign, M, Fx * self.mid_span * sign  # roll moment, pitch moment, yaw moment due to drag at panel mid-span

        return (forces_body, moments_body)


    def local_airflow(self, state: StateVector, relative_airflow: V3d, sign: float) -> V3d:
        """
        Calculate local airflow at panel due to angular velocity. 
        Retreating wing has reduced local airflow (), advancing wing has increased local airflow.
        Downgoing wing has increased local airflow, upgoing wing has reduced local airflow.
        
        Args:
            state: Current state vector
            relative_airflow: Relative airflow vector [u, v, w] in body frame
            sign: +1 for right wing, -1 for left wing
        
        Returns:
            local_airflow: Local airflow vector [u, v, w] at panel in body frame
        """
        dz = state.angular_velocity()[0] * self.mid_span * sign  # roll rate * mid-span point of panel. Difference in local airflow due to roll rate
        dx = -state.angular_velocity()[2] * self.mid_span * sign # yaw rate * mid-span point of panel.  Difference in local airflow due to yaw rate

        local_airflow = relative_airflow[0] + dx, relative_airflow[1], relative_airflow[2] - dz
        return local_airflow
    

    def coefficients_at(self, aoa: float) -> tuple[float, float, float]:
        """
        Get lift, drag, moment coefficients at given angle of attack.
        
        Args:
            aoa: Angle of attack in radians
        Returns:
            (Cl, Cd, Cm) - lift, drag, moment coefficients
        """
        rootFraction = 1.0 - self.interp
        tipFraction = self.interp

        root = self.rootFoil
        tip = self.tipFoil 

        Clr, Cdr, Cmr = root.coefficients_at(aoa)
        Clt, Cdt, Cmt = tip.coefficients_at(aoa)
        Cl = rootFraction * Clr + tipFraction * Clt
        Cd = rootFraction * Cdr + tipFraction * Cdt 
        Cm = rootFraction * Cmr + tipFraction * Cmt
        return (Cl, Cd, Cm)

    
class AileronPanel(Panel):
    """
    AileronPanel is a Panel with an aileron control surface.
    """
    def process(self, state: StateVector, relative_airflow: V3d, world: World, dihedral: float, controls: ControlInputs, sign: float) -> tuple[V3d, V3d]:

        # Get local airflow at panel due to angular velocity
        local_airflow = self.local_airflow(state, relative_airflow, sign)

        local_tas = TotalAirspeed(local_airflow)
        local_alpha = AngleOfAttack(local_airflow)
        beta = SideslipAngle(local_airflow)

        # Effective AoA for sideslip 
        aoa_beta = atan2(local_airflow[1], local_airflow[0])

        local_alpha += self.incidence  # add geometric incidence angle
        aoa_beta += dihedral  # add dihedral effect

        aoa = local_alpha * cos(beta) + aoa_beta * sin(beta)

        Cl, Cd, Cm = self.coefficients_at(aoa)

        q = 0.5 * world.air_density * local_tas**2
        L = Cl * q * self.area
        D = Cd * q * self.area
        M = Cm * q * self.area * self.quater_chord

        # convert L, D to body axes and sum
        # Transform to body axes
        Fx = -D * cos(aoa) + L * sin(aoa)
        Fz = -D * sin(aoa) - L * cos(aoa)

        forces_body = Fx, 0.0, Fz  # drag in body X, side force 0, lift in body z
        

        # Moments (about c.g.)
        # Assume wing is centered on fuselage centerline (no spanwise moment)
        # Roll moment due to lift at panel mid-span
        moments_body = Fz * self.mid_span * sign, M, Fx * self.mid_span * sign  # roll moment, pitch moment, yaw moment due to drag at panel mid-span

        return (forces_body, moments_body)

class AirbrakePanel(Panel):
    """
    AirbrakePanel is a Panel with an airbrake control surface.
    """
    def process(self, state: StateVector, relative_airflow: V3d, world: World, dihedral: float, controls: ControlInputs, sign: float) -> tuple[V3d, V3d]:

        # Get local airflow at panel due to angular velocity
        local_airflow = self.local_airflow(state, relative_airflow, sign)

        local_tas = TotalAirspeed(local_airflow)
        local_alpha = AngleOfAttack(local_airflow)
        beta = SideslipAngle(local_airflow)

        # Effective AoA for sideslip 
        aoa_beta = atan2(local_airflow[1], local_airflow[0])

        local_alpha += self.incidence  # add geometric incidence angle
        aoa_beta += dihedral  # add dihedral effect

        aoa = local_alpha * cos(beta) + aoa_beta * sin(beta)

        Cl, Cd, Cm = self.coefficients_at(aoa)

        q = 0.5 * world.air_density * local_tas**2
        L = Cl * q * self.area
        D = Cd * q * self.area
        M = Cm * q * self.area * self.quater_chord

        # convert L, D to body axes and sum
        # Transform to body axes
        Fx = -D * cos(aoa) + L * sin(aoa)
        Fz = -D * sin(aoa) - L * cos(aoa)

        forces_body = Fx, 0.0, Fz  # drag in body X, side force 0, lift in body z
        

        # Moments (about c.g.)
        # Assume wing is centered on fuselage centerline (no spanwise moment)
        # Roll moment due to lift at panel mid-span
        moments_body = Fz * self.mid_span * sign, M, Fx * self.mid_span * sign # roll moment, pitch moment, yaw moment due to drag at panel mid-span

        return (forces_body, moments_body)
