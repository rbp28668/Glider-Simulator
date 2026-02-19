import math
from aerofoil import Aerofoil
from math import atan2, cos, radians, sin, isnan, isinf

from aircraft_params import AircraftParameters
from control_inputs import ControlInputs
from state_vector import StateVector
from v3d import AngleOfAttack, SideslipAngle, TotalAirspeed, V3d
from world import World

# Minimum airspeed for aerodynamic calculations (m/s)
# Below this, forces are scaled to zero to prevent numerical instability
MIN_AIRSPEED = 1.0

# Maximum force magnitude per panel (N) - prevents runaway
MAX_PANEL_FORCE = 50000.0


def clamp(value: float, min_val: float, max_val: float) -> float:
    """Clamp value to range [min_val, max_val]."""
    return max(min_val, min(max_val, value))


def safe_value(value: float, default: float = 0.0) -> float:
    """Return default if value is NaN or Inf."""
    if isnan(value) or isinf(value):
        return default
    return value


class Panel:
    """
    Panel is part of a wing. It may be extended to include control surfaces such as ailerons or airbrakes.

    Subclasses can override hook methods to modify behavior:
    - modify_aoa(): Adjust angle of attack (e.g., for ailerons)
    - modify_coefficients(): Adjust Cl, Cd, Cm (e.g., for spoiler lift reduction)
    - additional_drag(): Add extra drag (e.g., for deployed airbrakes)
    """
    def __init__(self, area: float, mid_span: float, quater_chord: float, mean_chord: float,
                 incidenceDegrees: float, rootFoil: Aerofoil, tipFoil: Aerofoil, interp: float = 0.0):
        self.area = area
        self.mid_span = mid_span
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

        # Protection against very low airspeed (stall/spin conditions)
        if local_tas < MIN_AIRSPEED:
            # Scale forces smoothly to zero as airspeed drops
            airspeed_factor = local_tas / MIN_AIRSPEED
            local_tas = MIN_AIRSPEED  # Prevent division issues
        else:
            airspeed_factor = 1.0

        local_alpha = AngleOfAttack(local_velocity)
        beta = SideslipAngle(local_velocity)

        # Effective AoA for sideslip
        aoa_beta = atan2(local_velocity[1], local_velocity[0])

        aoa = local_alpha + self.incidence  # add geometric incidence angle

        # Hook: allow subclasses to modify AoA (e.g., aileron deflection)
        aoa = self.modify_aoa(aoa, controls, sign)

        # Dihedral effect: when slipping right (beta > 0), right wing sees increased AoA,
        # left wing sees decreased AoA. This creates restoring roll moment (Cl_beta).
        # The sign parameter differentiates right (+1) from left (-1) wing.
        # Simple linear model: delta_aoa = dihedral * beta * sign
        # Limited to prevent runaway at extreme sideslip
        MAX_DIHEDRAL_BETA = 0.35  # ~20 degrees - beyond this, flow is separated/nonlinear
        beta_limited = max(-MAX_DIHEDRAL_BETA, min(MAX_DIHEDRAL_BETA, beta))
        aoa += aircraft.dihedral_angle * beta_limited * sign

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
        Fx = -(D * cos(local_alpha) - L * sin(local_alpha))  # drag backwards in S&L flight
        Fz = -(D * sin(local_alpha) + L * cos(local_alpha))  # lift is -ve Z in body axes

        # Apply low-airspeed scaling
        Fx *= airspeed_factor
        Fz *= airspeed_factor
        M *= airspeed_factor

        # Clamp forces to prevent numerical instability
        Fx = clamp(safe_value(Fx), -MAX_PANEL_FORCE, MAX_PANEL_FORCE)
        Fz = clamp(safe_value(Fz), -MAX_PANEL_FORCE, MAX_PANEL_FORCE)
        M = clamp(safe_value(M), -MAX_PANEL_FORCE * 10, MAX_PANEL_FORCE * 10)

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
        p = state.angular_velocity()[0]
        r = state.angular_velocity()[2]

        # Change in z velocity. If rolling right, panel going down and Z increasing
        dz = p * self.mid_span * sign
        # Change in x velocity. If yawing right, right panel retreating and X decreasing
        dx = -r * self.mid_span * sign

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
    """Panel with aileron control surface.

    Models aileron effects including:
    - Camber change affecting lift (via effective AoA shift)
    - Pitching moment change due to camber
    - Differential deflection (up vs down travel)
    - Profile drag from deflection

    Thin airfoil theory: A plain flap deflection changes:
    - Zero-lift angle: Δα_0 ≈ -ε * δ (ε = lift_effectiveness, typically 0.5-0.7)
    - Pitching moment: ΔCm = moment_coeff * δ (typically -0.3 to -0.5 per radian)
    """

    def __init__(self, area: float, mid_span: float, quater_chord: float, mean_chord: float,
                 incidenceDegrees: float, rootFoil: Aerofoil, tipFoil: Aerofoil, interp: float = 0.0,
                 max_up_deg: float = 5.0, max_down_deg: float = 5.0,
                 lift_effectiveness: float = 0.6, moment_coeff: float = -0.4,
                 profile_drag_coeff: float = 0.01):
        """
        Args:
            lift_effectiveness: Fraction of deflection that acts as AoA change for lift.
                               Thin airfoil theory gives ~0.5-0.7 for typical aileron chord ratios.
            moment_coeff: Change in Cm per radian of deflection (negative = nose down for
                         trailing-edge-down deflection). Typical range -0.3 to -0.5.
            profile_drag_coeff: Drag coefficient per radian² of deflection.
        """
        super().__init__(area, mid_span, quater_chord, mean_chord, incidenceDegrees, rootFoil, tipFoil, interp)
        self.max_up = radians(max_up_deg)      # max deflection for up-going aileron
        self.max_down = radians(max_down_deg)  # max deflection for down-going aileron
        self.lift_effectiveness = lift_effectiveness  # how much deflection changes effective AoA
        self.moment_coeff = moment_coeff              # ΔCm per radian of deflection
        self.profile_drag_coeff = profile_drag_coeff  # drag coefficient per radian² of deflection
        self._last_deflection = 0.0  # store for drag and moment calculation

    def modify_aoa(self, aoa: float, controls: ControlInputs, sign: float) -> float:
        """Aileron deflection changes effective angle of attack (camber effect on lift).

        Differential: up-going aileron can deflect more than down-going.
        - controls.roll * sign > 0: aileron goes up (reduces AoA/lift)
        - controls.roll * sign < 0: aileron goes down (increases AoA/lift)

        The lift_effectiveness factor accounts for the fact that a plain flap
        is less effective at changing lift than a pure AoA change.
        """
        command = controls.roll * sign
        if command >= 0:
            # Up-going aileron (reduces lift on this wing)
            deflection = command * self.max_up
        else:
            # Down-going aileron (increases lift on this wing)
            deflection = command * self.max_down

        self._last_deflection = deflection
        # Apply lift effectiveness - deflection is less effective than pure AoA change
        return aoa - deflection * self.lift_effectiveness

    def modify_coefficients(self, Cl: float, Cd: float, Cm: float,
                            controls: ControlInputs) -> tuple[float, float, float]:
        """Modify pitching moment due to aileron camber change.

        Trailing-edge-down deflection (positive δ) creates nose-down moment (negative ΔCm).
        This is because the aft camber increase shifts the center of pressure rearward.
        """
        # Moment change due to camber: ΔCm = moment_coeff * δ
        # Note: _last_deflection is positive for up-aileron (reduces camber)
        # So we negate it: down deflection should give negative ΔCm
        delta_Cm = self.moment_coeff * (-self._last_deflection)
        return Cl, Cd, Cm + delta_Cm

    def additional_drag(self, q: float, controls: ControlInputs) -> float:
        """Deflected aileron adds profile drag proportional to deflection²."""
        # Drag increment: Cd = k * δ²
        Cd_aileron = self.profile_drag_coeff * self._last_deflection * self._last_deflection
        return Cd_aileron * q * self.area


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
