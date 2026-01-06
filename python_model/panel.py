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
    def __init__(self, span: float, area: float, quater_chord: float, mean_chord: float,  incidenceDegrees: float, rootFoil : Aerofoil , tipFoil : Aerofoil, interp : float = 0.0):
        self.mid_span = span
        self.area = area
        self.quater_chord = quater_chord
        self.incidence = radians(incidenceDegrees)
        self.rootFoil = rootFoil
        self.tipFoil = tipFoil
        self.interp = interp  # interpolation factor between root and tip aerofoils (0.0 = root, 1.0 = tip)
        self.mean_chord = mean_chord



    def process(self, state: StateVector, relative_velocity: V3d, cg: float, world: World, dihedral: float, controls: ControlInputs, sign: float) -> tuple[V3d, V3d]:
        """
        Process the panel to calculate forces and moments.
        Args:
            state: Current state vector
            relative_velocity: The aircraft velocity relative to the air around it (in body axes, wind corrected)
            cg: Center of gravity position (m from datum)
            world: The simulation world
            dihedral: Dihedral angle of the wing (radians)
            controls: Current control surface deflections
            sign: +1 for right wing, -1 for left wing

        Returns:
            forces_body: [Fx, Fy, Fz] (N)
            moments_body: [L, M, N] (N·m)
        """

        # Get local airflow at panel due to angular velocity
        local_velocity = self.get_local_velocity(state, relative_velocity, sign)

        local_tas = TotalAirspeed(local_velocity)
        local_alpha = AngleOfAttack(local_velocity)
        beta = SideslipAngle(local_velocity)

        # Effective AoA for sideslip 
        aoa_beta = atan2(local_velocity[1], local_velocity[0])

        local_alpha += self.incidence  # add geometric incidence angle
        aoa_beta += dihedral  # add dihedral effect

        aoa = local_alpha * cos(beta) + aoa_beta * sin(beta)

        Cl, Cd, Cm = self.coefficients_at(aoa)

        q = 0.5 * world.air_density * local_tas**2
        L = Cl * q * self.area
        D = Cd * q * self.area
        M = Cm * q * self.area * self.mean_chord

        # convert L, D to body axes and sum
        # Transform to body axes
        Fx = -D * cos(aoa) - L * sin(aoa)   # drag backwards in S&L flight
        Fz = -D * sin(aoa) - L * cos(aoa)   # lift is -ve Z in body axes

        forces_body = Fx, 0.0, Fz  # drag in body X, side force 0, lift in body z
        
        dist = self.quater_chord - cg       # calculae moments from c.g. not datum.

        # Moments (about c.g.)
        # Assume wing is centered on fuselage centerline (no spanwise moment)
        # Roll moment due to lift at panel mid-span
        moments_body = (
            Fz * self.mid_span * sign,  
            M - Fz * dist,  # pitching moment about c.g., (if quater_chord behind cg then dist is -ve, upward lift also -ve, so nose-down moment)
            -Fx * self.mid_span * sign  # yaw moment, drag -ve at panel mid-span,  yaw moment positive to right
            )  # roll moment, pitch moment, yaw moment due to drag at panel mid-span

        return (forces_body, moments_body)


    def get_local_velocity(self, state: StateVector, relative_velocity: V3d, sign: float) -> V3d:
        """
        Calculate local velocity at panel due to angular velocity. 
        Retreating wing has reduced local airflow (), advancing wing has increased local airflow.
        Downgoing wing has increased local airflow, upgoing wing has reduced local airflow.
        
        Args:
            state: Current state vector
            relative_velocity: Relative airframe velocity vector [u, v, w] in body frame relative to air-mass
            sign: +1 for right wing, -1 for left wing
        
        Returns:
            local_airflow: Local airflow vector [u, v, w] at panel in body frame.  Note that this is actually
            the velocity of the centre of the panel relative to the stationary airmass.
        """

        # Change in z velocity.   If rolling right, panel going down and Z increasing 
        dz = state.angular_velocity()[0] * self.mid_span * sign  # roll rate * mid-span point of panel. Difference in local airflow due to roll rate
        # Change in x velocity.  If yawing right, right panel retreating and X decreasing
        dx = -state.angular_velocity()[2] * self.mid_span * sign # yaw rate * mid-span point of panel.  Difference in local airflow due to yaw rate

        local_airflow = relative_velocity[0] + dx, relative_velocity[1], relative_velocity[2] + dz
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
    def process(self, state: StateVector, relative_velocity: V3d, cg: float, world: World, dihedral: float, controls: ControlInputs, sign: float) -> tuple[V3d, V3d]:
        """
        Process the panel to calculate forces and moments.
        Args:
            state: Current state vector
            relative_velocity: The aircraft velocity relative to the air around it (in body axes, wind corrected)
            cg: Center of gravity position (m from datum)
            world: The simulation world
            dihedral: Dihedral angle of the wing (radians)
            controls: Current control surface deflections
            sign: +1 for right wing, -1 for left wing

        Returns:
            forces_body: [Fx, Fy, Fz] (N)
            moments_body: [L, M, N] (N·m)
        """

        # Get local airflow at panel due to angular velocity
        local_velocity = self.get_local_velocity(state, relative_velocity, sign)

        local_tas = TotalAirspeed(local_velocity)
        local_alpha = AngleOfAttack(local_velocity)
        beta = SideslipAngle(local_velocity)

        # Effective AoA for sideslip 
        aoa_beta = atan2(local_velocity[1], local_velocity[0])

        local_alpha += self.incidence  # add geometric incidence angle
        local_alpha -= controls.roll * radians(5) * sign  # TODO properly - aileron effect.  Roll right, reduce AoA on right wing (+sign), increase AoA on left wing (-sign)
        aoa_beta += dihedral  # add dihedral effect

        aoa = local_alpha * cos(beta) + aoa_beta * sin(beta)

        Cl, Cd, Cm = self.coefficients_at(aoa)

        q = 0.5 * world.air_density * local_tas**2
        L = Cl * q * self.area
        D = Cd * q * self.area
        M = Cm * q * self.area * self.mean_chord

        # convert L, D to body axes and sum
        # Transform to body axes
        Fx = -D * cos(aoa) - L * sin(aoa)   # drag backwards in S&L flight
        Fz = -D * sin(aoa) - L * cos(aoa)   # lift is -ve Z in body axes

        forces_body = Fx, 0.0, Fz  # drag in body X, side force 0, lift in body z
        
        dist = self.quater_chord - cg       # calculae moments from c.g. not datum.

        # Moments (about c.g.)
        # Assume wing is centered on fuselage centerline (no spanwise moment)
        # Roll moment due to lift at panel mid-span
        moments_body = (
            Fz * self.mid_span * sign,  
            M - Fz * dist,  # pitching moment about c.g., (if quater_chord behind cg then dist is -ve, upward lift also -ve, so nose-down moment)
            -Fx * self.mid_span * sign  # yaw moment, drag -ve at panel mid-span,  yaw moment positive to right
            )  # roll moment, pitch moment, yaw moment due to drag at panel mid-span

        return (forces_body, moments_body)

class AirbrakePanel(Panel):
    """
    AirbrakePanel is a Panel with an airbrake control surface.
    """
def process(self, state: StateVector, relative_velocity: V3d, cg: float, world: World, dihedral: float, controls: ControlInputs, sign: float) -> tuple[V3d, V3d]:
        """
        Process the panel to calculate forces and moments.
        Args:
            state: Current state vector
            relative_velocity: The aircraft velocity relative to the air around it (in body axes, wind corrected)
            cg: Center of gravity position (m from datum)
            world: The simulation world
            dihedral: Dihedral angle of the wing (radians)
            controls: Current control surface deflections
            sign: +1 for right wing, -1 for left wing

        Returns:
            forces_body: [Fx, Fy, Fz] (N)
            moments_body: [L, M, N] (N·m)
        """

        # Get local airflow at panel due to angular velocity
        local_velocity = self.local_airflow(state, relative_velocity, sign)

        local_tas = TotalAirspeed(local_velocity)
        local_alpha = AngleOfAttack(local_velocity)
        beta = SideslipAngle(local_velocity)

        # Effective AoA for sideslip 
        aoa_beta = atan2(local_velocity[1], local_velocity[0])

        local_alpha += self.incidence  # add geometric incidence angle
        aoa_beta += dihedral  # add dihedral effect

        aoa = local_alpha * cos(beta) + aoa_beta * sin(beta)

        Cl, Cd, Cm = self.coefficients_at(aoa)

        # Modify Cl based on airbrake extension (Crude!)
        Cl *= 1-0.8 * controls.spoilers  # reduce lift with airbrake extension.  (est)

        q = 0.5 * world.air_density * local_tas**2
        L = Cl * q * self.area
        D = Cd * q * self.area
        M = Cm * q * self.area * self.mean_chord

        # Now add in drag for spoiler
        Cd_spoiler = 1.8   # flat plate
        spoiler_area = 0.3 * self.area   # area of airbrake about 1/3 panel area?
        d_spoiler = Cd_spoiler * q * spoiler_area * controls.spoilers
        D += d_spoiler

        # convert L, D to body axes and sum
        # Transform to body axes
        Fx = -D * cos(aoa) - L * sin(aoa)   # drag backwards in S&L flight
        Fz = -D * sin(aoa) - L * cos(aoa)   # lift is -ve Z in body axes

        forces_body = Fx, 0.0, Fz  # drag in body X, side force 0, lift in body z
        
        dist = self.quater_chord - cg       # calculae moments from c.g. not datum.

        # Moments (about c.g.)
        # Assume wing is centered on fuselage centerline (no spanwise moment)
        # Roll moment due to lift at panel mid-span
        moments_body = (
            Fz * self.mid_span * sign,  
            M - Fz * dist,  # pitching moment about c.g., (if quater_chord behind cg then dist is -ve, upward lift also -ve, so nose-down moment)
            -Fx * self.mid_span * sign  # yaw moment, drag -ve at panel mid-span,  yaw moment positive to right
            )  # roll moment, pitch moment, yaw moment due to drag at panel mid-span

        return (forces_body, moments_body)
