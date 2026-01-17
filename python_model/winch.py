"""
Winch launch simulation for glider.

Models a ground-based winch that reels in a cable attached to the glider's
winch hook, providing the force for a winch launch.

Features:
- Cable tension based on winch power and drum speed
- Automatic back-release if cable pulls backwards relative to glider
- Weak link simulation (optional)
- Cable runs out detection
"""

from math import sqrt, acos, pi
from state_vector import StateVector
from contact_point import ContactPoint
from quaternion import quaternion_rotate_vector, quaternion_rotate_vector_inverse

# Type alias
V3d = tuple[float, float, float]


class Winch:
    """
    Winch launch simulation.

    The winch is positioned at a fixed location on the ground and reels in
    a cable attached to the glider's winch hook. The cable applies a tension
    force along its length from the hook towards the winch.

    Back-release: If the cable direction has a negative X component in the
    glider's body frame (pulling backwards), the cable automatically releases.
    """

    def __init__(self,
                 winch_position: V3d = (1000.0, 0.0, 0.0),
                 max_tension: float = 9000.0,
                 weak_link: float = 10000.0,
                 cable_length: float = 1500.0):
        """
        Initialize winch.

        Args:
            winch_position: (X, Y, Z) position of winch drum in earth frame (m)
                           Default 1000m ahead (north), on ground
            max_tension: Maximum cable tension from winch power (N)
                        Default 9000N (~1.3x glider weight for good climb)
            weak_link: Tension at which weak link breaks (N)
                      Default 10000N
            cable_length: Total cable length on drum (m)
                         Default 1500m
        """
        self.winch_position = winch_position
        self.max_tension = max_tension
        self.weak_link = weak_link
        self.cable_length = cable_length

        # State
        self.engaged = False
        self.cable_out = 0.0  # Length of cable paid out (m)
        self.tension = 0.0    # Current cable tension (N)
        self.release_reason = None  # Why cable was released

        # Cable properties
        self.cable_stiffness = 50000.0  # N/m - cable elasticity
        self.cable_damping = 1000.0     # N.s/m - cable damping

        # Winch drum model
        self.drum_speed = 30.0  # Target reel-in speed (m/s)
        self.drum_power = 335000.0  # Winch power (W) - ~450 HP

    def engage(self, initial_cable_out: float = None):
        """
        Engage the winch cable.

        Args:
            initial_cable_out: Initial cable length paid out (m)
                              If None, calculated from glider position
        """
        self.engaged = True
        self.release_reason = None
        if initial_cable_out is not None:
            self.cable_out = initial_cable_out

    def release(self, reason: str = "manual"):
        """
        Release the cable.

        Args:
            reason: Why the cable was released
        """
        self.engaged = False
        self.tension = 0.0
        self.release_reason = reason

    def calculate_forces(self,
                        state: StateVector,
                        hook_position_body: V3d,
                        dt: float) -> tuple[V3d, V3d, dict]:
        """
        Calculate winch cable forces and moments.

        Args:
            state: Current aircraft state
            hook_position_body: Winch hook position in body frame (m)
            dt: Time step (s)

        Returns:
            forces_body: Cable force in body frame (Fx, Fy, Fz) in N
            moments_body: Moment about CG in body frame (L, M, N) in N.m
            info: Dictionary with diagnostic information
        """
        info = {
            'engaged': self.engaged,
            'cable_out': self.cable_out,
            'tension': 0.0,
            'cable_angle_deg': 0.0,
            'back_release': False,
            'weak_link_break': False,
            'cable_run_out': False
        }

        # No force if not engaged
        if not self.engaged:
            return (0.0, 0.0, 0.0), (0.0, 0.0, 0.0), info

        # Get hook position in earth frame
        orientation = state.orientation()
        hook_earth_offset = quaternion_rotate_vector(orientation, hook_position_body)
        aircraft_pos = state.position()

        hook_earth = (
            aircraft_pos[0] + hook_earth_offset[0],
            aircraft_pos[1] + hook_earth_offset[1],
            aircraft_pos[2] + hook_earth_offset[2]
        )

        # Vector from hook to winch (cable direction)
        cable_vec = (
            self.winch_position[0] - hook_earth[0],
            self.winch_position[1] - hook_earth[1],
            self.winch_position[2] - hook_earth[2]
        )

        # Cable length (distance from hook to winch)
        cable_distance = sqrt(cable_vec[0]**2 + cable_vec[1]**2 + cable_vec[2]**2)

        if cable_distance < 1.0:
            # Too close to winch, release
            self.release("cable_run_out")
            info['cable_run_out'] = True
            return (0.0, 0.0, 0.0), (0.0, 0.0, 0.0), info

        # Unit vector along cable (from hook towards winch)
        cable_unit = (
            cable_vec[0] / cable_distance,
            cable_vec[1] / cable_distance,
            cable_vec[2] / cable_distance
        )

        # Transform cable direction to body frame
        cable_body = quaternion_rotate_vector_inverse(orientation, cable_unit)

        # Check for back-release: cable pulling backwards (negative X in body frame)
        if cable_body[0] < 0:
            self.release("back_release")
            info['back_release'] = True
            return (0.0, 0.0, 0.0), (0.0, 0.0, 0.0), info

        # Calculate cable angle from horizontal (for info)
        horizontal_dist = sqrt(cable_vec[0]**2 + cable_vec[1]**2)
        if horizontal_dist > 0.1:
            cable_angle = acos(min(1.0, horizontal_dist / cable_distance))
            info['cable_angle_deg'] = cable_angle * 180.0 / pi

        # Update cable out length
        self.cable_out = cable_distance
        info['cable_out'] = self.cable_out

        # Check if cable has run out
        if self.cable_out > self.cable_length:
            self.release("cable_run_out")
            info['cable_run_out'] = True
            return (0.0, 0.0, 0.0), (0.0, 0.0, 0.0), info

        # Calculate tension using power-limited winch model
        # Power = Force * velocity
        # At the hook, velocity component along cable determines power delivery

        # Get hook velocity in earth frame
        hook_vel_body = self._get_hook_velocity(state, hook_position_body)
        hook_vel_earth = quaternion_rotate_vector(orientation, hook_vel_body)

        # Velocity component along cable (positive = towards winch = good)
        v_cable = (
            hook_vel_earth[0] * cable_unit[0] +
            hook_vel_earth[1] * cable_unit[1] +
            hook_vel_earth[2] * cable_unit[2]
        )

        # Winch drum tries to reel in at drum_speed
        # Tension is based on difference between drum speed and actual cable speed
        speed_error = self.drum_speed - v_cable

        # Simple tension model: tension proportional to speed error, limited by power
        if speed_error > 0:
            # Cable moving slower than drum wants - apply tension
            # Power limited: T * v <= Power, so T <= Power / v
            if v_cable > 1.0:
                power_limited_tension = self.drum_power / v_cable
            else:
                power_limited_tension = self.max_tension

            self.tension = min(self.max_tension, power_limited_tension)
        else:
            # Cable moving faster than drum - minimal tension (cable slack)
            self.tension = 100.0  # Small tension to keep cable taut

        # Check weak link
        if self.tension > self.weak_link:
            self.release("weak_link")
            info['weak_link_break'] = True
            return (0.0, 0.0, 0.0), (0.0, 0.0, 0.0), info

        info['tension'] = self.tension

        # Apply force along cable direction (in earth frame, towards winch)
        force_earth = (
            self.tension * cable_unit[0],
            self.tension * cable_unit[1],
            self.tension * cable_unit[2]
        )

        # Transform force to body frame
        force_body = quaternion_rotate_vector_inverse(orientation, force_earth)

        # Calculate moment about CG
        # Hook position relative to CG (assuming CG at datum for now)
        # The simulation will need to pass CG offset, but for now use hook_position_body
        arm = hook_position_body

        # Moment = arm x force
        moment_body = (
            arm[1] * force_body[2] - arm[2] * force_body[1],
            arm[2] * force_body[0] - arm[0] * force_body[2],
            arm[0] * force_body[1] - arm[1] * force_body[0]
        )

        return force_body, moment_body, info

    def _get_hook_velocity(self, state: StateVector, hook_body: V3d) -> V3d:
        """
        Calculate velocity of hook point in body frame.

        V_hook = V_cg + omega x r_hook
        """
        u, v, w = state.velocity()
        p, q, r = state.angular_velocity()

        # omega x r (cross product)
        omega_cross_r = (
            q * hook_body[2] - r * hook_body[1],
            r * hook_body[0] - p * hook_body[2],
            p * hook_body[1] - q * hook_body[0]
        )

        return (
            u + omega_cross_r[0],
            v + omega_cross_r[1],
            w + omega_cross_r[2]
        )

    def get_status(self) -> dict:
        """Get current winch status."""
        return {
            'engaged': self.engaged,
            'tension': self.tension,
            'cable_out': self.cable_out,
            'release_reason': self.release_reason
        }
