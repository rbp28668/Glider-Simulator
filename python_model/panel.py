import math
from aerofoil import Aerofoil
from math import atan2, cos, radians, sin

from aircraft_params import AircraftParameters
from control_inputs import ControlInputs
from state_vector import StateVector
from v3d import AngleOfAttack, SideslipAngle, TotalAirspeed, V3d
from world import World


class Panel:
    """
    Panel is part of a wing. It may be extended to include control surfaces such as ailerons or airbrakes.

    Subclasses can override hook methods to modify behavior:
    - modify_aoa(): Adjust angle of attack (e.g., for ailerons)
    - modify_coefficients(): Adjust Cl, Cd, Cm (e.g., for spoiler lift reduction)
    - additional_drag(): Add extra drag (e.g., for deployed airbrakes)
    """
    def __init__(self, span: float, area: float, quater_chord: float, mean_chord: float,
                 incidenceDegrees: float, rootFoil: Aerofoil, tipFoil: Aerofoil, interp: float = 0.0):
        self.mid_span = span
        self.area = area
        self.quater_chord = quater_chord
        self.incidence = radians(incidenceDegrees)
        self.rootFoil = rootFoil
        self.tipFoil = tipFoil
        self.interp = interp  # interpolation factor between root and tip aerofoils (0.0 = root, 1.0 = tip)
        self.mean_chord = mean_chord

    def process(self, state: StateVector, relative_velocity: V3d, aircraft: AircraftParameters,
                world: World, controls: ControlInputs, sign: float) -> tuple[V3d, V3d]:
        """
        Process the panel to calculate forces and moments.

        Args:
            state: Current state vector
            relative_velocity: The aircraft velocity relative to the air around it (in body axes, wind corrected)
            aircraft: Aircraft parameters including CG position
            world: The simulation world
            controls: Current control surface deflections
            sign: +1 for right wing, -1 for left wing

        Returns:
            forces_body: [Fx, Fy, Fz] (N)
            moments_body: [L, M, N] (N.m)
        """
        # Get local airflow at panel due to angular velocity
        local_velocity = self.get_local_velocity(state, relative_velocity, sign)

        local_tas = TotalAirspeed(local_velocity)
        local_alpha = AngleOfAttack(local_velocity)
        beta = SideslipAngle(local_velocity)

        # Effective AoA for sideslip
        aoa_beta = atan2(local_velocity[1], local_velocity[0])

        aoa = local_alpha + self.incidence  # add geometric incidence angle

        # Hook: allow subclasses to modify AoA (e.g., aileron deflection)
        aoa = self.modify_aoa(aoa, controls, sign)

        aoa_beta += aircraft.dihedral_angle  # add dihedral effect
        aoa = aoa * cos(beta) + aoa_beta * sin(beta)

        Cl, Cd, Cm = self.coefficients_at(aoa)

        # Hook: allow subclasses to modify coefficients (e.g., spoiler lift reduction)
        Cl, Cd, Cm = self.modify_coefficients(Cl, Cd, Cm, controls)

        # Lift dependent drag
        Cdi = (Cl * Cl) / (math.pi * aircraft.AR * aircraft.oswald)
        Cd += Cdi

        q = 0.5 * world.air_density * local_tas**2
        L = Cl * q * self.area
        D = Cd * q * self.area
        M = Cm * q * self.area * self.mean_chord

        # Hook: allow subclasses to add extra drag (e.g., deployed airbrakes)
        D += self.additional_drag(q, controls)

        # Transform from wind axes to body axes (rotation by angle of attack about Y)
        # Wind axes: -X is drag direction, -Z is lift direction
        # Body axes: X forward, Z down
        Fx = -D * cos(local_alpha) - L * sin(local_alpha)  # drag backwards in S&L flight
        Fz =  D * sin(local_alpha) - L * cos(local_alpha)  # lift is -ve Z in body axes

        forces_body = Fx, 0.0, Fz  # drag in body X, side force 0, lift in body Z

        dist = self.quater_chord - aircraft.CG  # calculate moments from c.g. not datum

        # Moments (about c.g.)
        # Roll moment due to lift at panel mid-span
        moments_body = (
            Fz * self.mid_span * sign,                # roll moment
            M - Fz * dist,                            # pitch moment about c.g.
            -Fx * self.mid_span * sign                # yaw moment due to drag
        )

        return (forces_body, moments_body)

    # --- Hook methods for subclasses to override ---

    def modify_aoa(self, aoa: float, controls: ControlInputs, sign: float) -> float:
        """Hook: modify angle of attack based on control inputs. Override in subclasses."""
        return aoa

    def modify_coefficients(self, Cl: float, Cd: float, Cm: float,
                            controls: ControlInputs) -> tuple[float, float, float]:
        """Hook: modify aerodynamic coefficients. Override in subclasses."""
        return Cl, Cd, Cm

    def additional_drag(self, q: float, controls: ControlInputs) -> float:
        """Hook: return additional drag force (N). Override in subclasses."""
        return 0.0

    # --- Utility methods ---

    def get_local_velocity(self, state: StateVector, relative_velocity: V3d, sign: float) -> V3d:
        """
        Calculate local velocity at panel due to angular velocity.
        Retreating wing has reduced local airflow, advancing wing has increased local airflow.
        Downgoing wing has increased local airflow, upgoing wing has reduced local airflow.

        Args:
            state: Current state vector
            relative_velocity: Relative airframe velocity vector [u, v, w] in body frame relative to air-mass
            sign: +1 for right wing, -1 for left wing

        Returns:
            local_airflow: Local airflow vector [u, v, w] at panel in body frame.
        """
        # Change in z velocity. If rolling right, panel going down and Z increasing
        dz = state.angular_velocity()[0] * self.mid_span * sign
        # Change in x velocity. If yawing right, right panel retreating and X decreasing
        dx = -state.angular_velocity()[2] * self.mid_span * sign

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

        Clr, Cdr, Cmr = self.rootFoil.coefficients_at(aoa)
        Clt, Cdt, Cmt = self.tipFoil.coefficients_at(aoa)

        Cl = rootFraction * Clr + tipFraction * Clt
        Cd = rootFraction * Cdr + tipFraction * Cdt
        Cm = rootFraction * Cmr + tipFraction * Cmt
        return (Cl, Cd, Cm)


class AileronPanel(Panel):
    """Panel with aileron control surface."""

    def modify_aoa(self, aoa: float, controls: ControlInputs, sign: float) -> float:
        """Aileron deflection changes effective angle of attack."""
        # Roll right: reduce AoA on right wing (+sign), increase AoA on left wing (-sign)
        # TODO: model this more accurately with camber change
        max_deflection = radians(5)
        return aoa - controls.roll * max_deflection * sign


class AirbrakePanel(Panel):
    """Panel with airbrake/spoiler control surface."""

    def modify_coefficients(self, Cl: float, Cd: float, Cm: float,
                            controls: ControlInputs) -> tuple[float, float, float]:
        """Airbrake deployment reduces lift coefficient."""
        # Crude model: reduce lift proportionally to spoiler deployment
        lift_reduction = 0.8 * controls.spoiler
        Cl *= (1.0 - lift_reduction)
        return Cl, Cd, Cm

    def additional_drag(self, q: float, controls: ControlInputs) -> float:
        """Deployed airbrake adds significant drag."""
        Cd_spoiler = 1.8          # flat plate drag coefficient
        spoiler_area = 0.3 * self.area  # airbrake is ~1/3 of panel area
        return Cd_spoiler * q * spoiler_area * controls.spoiler
