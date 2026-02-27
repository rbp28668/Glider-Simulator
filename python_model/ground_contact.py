"""
Ground contact physics for glider simulation.

Implements spring-damper normal forces and Coulomb friction model
for wheel, skid, and wingtip contacts.

Coordinate system notes:
- Body frame: X forward, Y right, Z down
- Earth frame: NED (X north, Y east, Z down)
- Ground height is Z coordinate in earth frame (Z=0 is ground level)
- Aircraft altitude is negative Z (flying at 1000m -> Z = -1000)
"""

from math import sqrt, copysign
from contact_point import ContactPoint
from state_vector import StateVector
from world import World
from quaternion import quaternion_rotate_vector, quaternion_rotate_vector_inverse
from v3d import V3d


# Default contact parameters by type
# Stiffness calculated for realistic penetration under load:
# - Main wheel: ~80% of 687kg weight at 0.10m max = 54kN -> 60kN/m
# - Nose/tail wheel: lighter load at 0.05m max -> 80kN/m
# - Wingtip: rarely loaded, 0.02m max -> 150kN/m
CONTACT_PARAMS = {
    'main_wheel': {
        'stiffness': 60000.0,       # N/m - main gear, most of weight
        'damping': 6000.0,          # N.s/m - damping coefficient
        'friction_static': 0.8,     # rubber on grass/asphalt
        'friction_dynamic': 0.6,
        'rolling_resistance': 0.03, # coefficient of rolling resistance
        'max_penetration': 0.10     # 10cm max
    },
    'nose_wheel': {
        'stiffness': 80000.0,       # N/m - nose wheel, lighter load
        'damping': 5000.0,
        'friction_static': 0.8,
        'friction_dynamic': 0.6,
        'rolling_resistance': 0.03,
        'max_penetration': 0.05     # 5cm max
    },
    'tail_wheel': {
        'stiffness': 80000.0,       # N/m - tail wheel/skid
        'damping': 5000.0,
        'friction_static': 0.5,     # often a skid
        'friction_dynamic': 0.4,
        'rolling_resistance': 0.10,
        'max_penetration': 0.05     # 5cm max
    },
    'wingtip': {
        'stiffness': 150000.0,      # N/m - stiff due to small max penetration
        'damping': 8000.0,
        'friction_static': 0.6,     # fiberglass on grass
        'friction_dynamic': 0.5,
        'rolling_resistance': 0.15,
        'max_penetration': 0.02     # 2cm max
    },
    # Legacy types for backwards compatibility
    'wheel': {
        'stiffness': 60000.0,
        'damping': 6000.0,
        'friction_static': 0.8,
        'friction_dynamic': 0.6,
        'rolling_resistance': 0.03,
        'max_penetration': 0.10
    },
    'skid': {
        'stiffness': 80000.0,
        'damping': 5000.0,
        'friction_static': 0.5,
        'friction_dynamic': 0.4,
        'rolling_resistance': 0.10,
        'max_penetration': 0.05
    }
}

# Velocity threshold for static/dynamic friction transition
FRICTION_VELOCITY_THRESHOLD = 0.1  # m/s

# Maximum force per contact point (numerical stability)
MAX_CONTACT_FORCE = 100000.0  # N

# Ground settling parameters - when aircraft is nearly stationary, damp strongly
SETTLE_VELOCITY_THRESHOLD = 0.5   # m/s - below this, apply settle damping
SETTLE_ANGULAR_THRESHOLD = 0.3    # rad/s - below this, apply settle damping
SNAP_TO_ZERO_THRESHOLD = 0.05     # m/s and rad/s - below this, force to zero


class ContactResult:
    """Result of a single contact point calculation."""
    def __init__(self):
        self.in_contact = False
        self.penetration = 0.0              # m (positive = into ground)
        self.normal_force = 0.0             # N (positive = pushing up)
        self.friction_force = (0.0, 0.0)    # (longitudinal, lateral) in N
        self.force_body = (0.0, 0.0, 0.0)   # Force in body frame
        self.moment_body = (0.0, 0.0, 0.0)  # Moment about CG in body frame
        self.position_earth = (0.0, 0.0, 0.0)  # Contact position in earth frame


class GroundContact:
    """
    Ground contact physics calculator.

    Implements spring-damper normal forces and Coulomb friction.
    """

    def __init__(self, cg_position: float):
        """
        Args:
            cg_position: CG x-coordinate relative to datum (negative = aft)
        """
        self.cg_position = cg_position

    def calculate_ground_forces(self,
                                state: StateVector,
                                contact_points: list[ContactPoint],
                                world: World,
                                cg_offset: float) -> tuple[V3d, V3d, list[ContactResult]]:
        """
        Calculate total ground contact forces and moments.

        Args:
            state: Current aircraft state
            contact_points: List of ContactPoint objects
            world: World object for ground height lookup
            cg_offset: CG position relative to datum (negative = aft)

        Returns:
            forces_body: Total force in body frame (Fx, Fy, Fz)
            moments_body: Total moment about CG in body frame (L, M, N)
            results: List of ContactResult for each contact point
        """
        total_force = [0.0, 0.0, 0.0]
        total_moment = [0.0, 0.0, 0.0]
        results = []
        contacts_in_ground = 0

        for cp in contact_points:
            result = self._calculate_single_contact(state, cp, world, cg_offset)
            results.append(result)

            if result.in_contact:
                contacts_in_ground += 1
                # Accumulate forces
                total_force[0] += result.force_body[0]
                total_force[1] += result.force_body[1]
                total_force[2] += result.force_body[2]

                # Accumulate moments
                total_moment[0] += result.moment_body[0]
                total_moment[1] += result.moment_body[1]
                total_moment[2] += result.moment_body[2]

        # Apply settling forces when 2+ contact points and low velocity
        # This prevents "fidgeting" when the aircraft should be stationary
        # (gliders typically sit on 2 points: main wheel + tail skid)
        if contacts_in_ground >= 2 :
            u, v, w = state.velocity()
            p, q, r = state.angular_velocity()
            vel_mag = sqrt(u*u + v*v + w*w)
            ang_mag = sqrt(p*p + q*q + r*r)

            if vel_mag < SETTLE_VELOCITY_THRESHOLD and ang_mag < SETTLE_ANGULAR_THRESHOLD:
                # Aircraft is nearly stationary - apply strong damping to settle
                # Use fixed high damping - this acts like friction/ground resistance
                settle_damp = 100000.0  # N.s/m - strong ground damping
                total_force[0] -= settle_damp * u
                total_force[1] -= settle_damp * v
                total_force[2] -= settle_damp * w

                # Angular damping: M = -c * omega
                settle_ang_damp = 50000.0  # N.m.s/rad
                total_moment[0] -= settle_ang_damp * p
                total_moment[1] -= settle_ang_damp * q
                total_moment[2] -= settle_ang_damp * r

                # Very low velocity - apply extreme damping to force to zero
                if vel_mag < SNAP_TO_ZERO_THRESHOLD and ang_mag < SNAP_TO_ZERO_THRESHOLD:
                    # Return special marker forces that simulation can detect
                    # Using very high damping to rapidly approach zero
                    total_force[0] -= 1000000.0 * u
                    total_force[1] -= 1000000.0 * v
                    total_force[2] -= 1000000.0 * w
                    total_moment[0] -= 500000.0 * p
                    total_moment[1] -= 500000.0 * q
                    total_moment[2] -= 500000.0 * r

        tf = total_force[0], total_force[1], total_force[2]
        tm = total_moment[0], total_moment[1], total_moment[2]
        return tf, tm, results

    def _calculate_single_contact(self,
                                  state: StateVector,
                                  cp: ContactPoint,
                                  world: World,
                                  cg_offset: float) -> ContactResult:
        """Calculate forces for a single contact point."""
        result = ContactResult()

        # Get contact parameters for this type
        params = self._get_contact_params(cp)

        # 1. Transform contact point from body frame to earth frame
        cp_body = cp.position_body()
        orientation = state.orientation()

        # Rotate contact point position to earth frame
        cp_earth_offset = quaternion_rotate_vector(orientation, cp_body)

        # Add aircraft CG position (state position is at datum, need to adjust)
        pos = state.position()
        cp_earth = (
            pos[0] + cp_earth_offset[0],
            pos[1] + cp_earth_offset[1],
            pos[2] + cp_earth_offset[2]
        )
        result.position_earth = cp_earth

        # 2. Get ground height at contact point
        ground_z = world.get_ground_height(cp_earth[0], cp_earth[1])

        # 3. Calculate penetration (positive = into ground)
        # In NED: Z is down, so penetration = cp_z - ground_z
        penetration = cp_earth[2] - ground_z

        # No contact if above ground
        if penetration < 0:
            return result

        result.in_contact = True
        max_pen = params.get('max_penetration', 0.10)
        result.penetration = penetration  # Store actual penetration

        # 4. Calculate contact point velocity in earth frame
        cp_vel_body = self._get_contact_velocity(state, cp, cg_offset)
        cp_vel_earth = quaternion_rotate_vector(orientation, cp_vel_body)

        # 5. Calculate normal force (spring-damper in earth Z direction)
        # F_normal = k * penetration - c * v_z
        k = params['stiffness']
        c = params['damping']

        # Vertical velocity component (positive = moving down)
        v_z = cp_vel_earth[2]

        # Normal force calculation
        # v_z > 0 means moving down, so damping should ADD force (oppose motion)
        if penetration <= max_pen:
            # Normal spring-damper within suspension travel
            F_normal = k * penetration + c * v_z
        else:
            # Beyond max penetration: add progressive stiffening
            # This simulates suspension bottoming out on a hard surface
            # Use quadratic stiffening for smoother response
            hard_stop_stiffness = 200000.0  # Stiff but not extreme (N/m)
            hard_stop_damping = 10000.0     # Additional damping for hard stop
            excess_penetration = penetration - max_pen
            # Quadratic stiffening: force increases rapidly with penetration
            F_normal = (k * max_pen +
                       hard_stop_stiffness * excess_penetration * (1 + 10 * excess_penetration) +
                       c * v_z +
                       hard_stop_damping * v_z)

        F_normal = max(0.0, F_normal)  # Can only push, not pull
        F_normal = min(F_normal, MAX_CONTACT_FORCE)
        result.normal_force = F_normal

        # 6. Calculate friction forces
        F_friction_x, F_friction_y = self._calculate_friction(
            cp_vel_earth, F_normal, params, cp.contact_type
        )
        result.friction_force = (F_friction_x, F_friction_y)

        # 7. Total force in earth frame
        # Normal force acts upward (-Z), friction acts in XY plane
        F_earth = (F_friction_x, F_friction_y, -F_normal)

        # 8. Transform force to body frame
        F_body = quaternion_rotate_vector_inverse(orientation, F_earth)
        result.force_body = F_body

        # 9. Calculate moment about CG
        # Moment arm from CG to contact point in body frame
        # CG is at (cg_offset, 0, 0) relative to datum
        arm = (
            cp.x - cg_offset,
            cp.y,
            cp.z
        )

        # Moment = arm x force (cross product)
        M_x = arm[1] * F_body[2] - arm[2] * F_body[1]  # Roll moment
        M_y = arm[2] * F_body[0] - arm[0] * F_body[2]  # Pitch moment
        M_z = arm[0] * F_body[1] - arm[1] * F_body[0]  # Yaw moment

        result.moment_body = (M_x, M_y, M_z)

        return result

    def _get_contact_velocity(self, state: StateVector, cp: ContactPoint,
                              cg_offset: float) -> V3d:
        """
        Calculate velocity of contact point in body frame.

        V_cp = V_cg + omega x r_cp
        where r_cp is vector from CG to contact point
        """
        u, v, w = state.velocity()
        p, q, r = state.angular_velocity()

        # Vector from CG to contact point (in body frame)
        # CG is at (cg_offset, 0, 0) relative to datum
        rx = cp.x - cg_offset
        ry = cp.y
        rz = cp.z

        # omega x r (cross product)
        omega_cross_r = (
            q * rz - r * ry,
            r * rx - p * rz,
            p * ry - q * rx
        )

        # Total velocity at contact point
        return (
            u + omega_cross_r[0],
            v + omega_cross_r[1],
            w + omega_cross_r[2]
        )

    def _calculate_friction(self,
                            vel_earth: V3d,
                            normal_force: float,
                            params: dict,
                            contact_type: str) -> tuple[float, float]:
        """
        Calculate friction forces using Coulomb model with velocity blending.

        Args:
            vel_earth: Contact point velocity in earth frame
            normal_force: Normal force magnitude (N)
            params: Contact parameters dict
            contact_type: 'wheel', 'skid', or 'wingtip'

        Returns:
            (F_x, F_y) friction forces in earth frame
        """
        # Horizontal velocity components
        v_x = vel_earth[0]
        v_y = vel_earth[1]
        v_horiz = sqrt(v_x**2 + v_y**2)

        if v_horiz < 1e-6 or normal_force < 1e-6:
            return (0.0, 0.0)

        # Determine friction coefficient
        mu_s = params['friction_static']
        mu_d = params['friction_dynamic']

        # Smooth transition between static and dynamic friction
        if v_horiz < FRICTION_VELOCITY_THRESHOLD:
            # Blend from static to dynamic
            blend = v_horiz / FRICTION_VELOCITY_THRESHOLD
            mu = mu_s * (1 - blend) + mu_d * blend
        else:
            mu = mu_d

        # Maximum friction force
        F_friction_max = mu * normal_force

        # Special handling for wheels (rolling vs sliding)
        if contact_type == 'wheel':
            # Longitudinal: rolling resistance (small, opposes motion)
            F_roll = params['rolling_resistance'] * normal_force
            # Smooth application of rolling resistance
            if abs(v_x) < FRICTION_VELOCITY_THRESHOLD:
                F_x = -v_x * (F_roll / FRICTION_VELOCITY_THRESHOLD)
            else:
                F_x = -copysign(F_roll, v_x)

            # Lateral: full friction (wheels don't roll sideways)
            if abs(v_y) < FRICTION_VELOCITY_THRESHOLD:
                # Proportional friction at low speed (prevents jitter)
                F_y = -v_y * (F_friction_max / FRICTION_VELOCITY_THRESHOLD)
            else:
                F_y = -copysign(F_friction_max, v_y)
        else:
            # Skids and wingtips: friction in both directions
            # Direction of friction opposes velocity
            F_x = -F_friction_max * (v_x / v_horiz)
            F_y = -F_friction_max * (v_y / v_horiz)

        # Clamp to max force
        F_x = max(-MAX_CONTACT_FORCE, min(MAX_CONTACT_FORCE, F_x))
        F_y = max(-MAX_CONTACT_FORCE, min(MAX_CONTACT_FORCE, F_y))

        return (F_x, F_y)

    def _get_contact_params(self, cp: ContactPoint) -> dict:
        """Get contact parameters, using overrides if specified."""
        base_params = CONTACT_PARAMS.get(cp.contact_type, CONTACT_PARAMS['wheel']).copy()

        # Apply any overrides from the contact point
        if cp.stiffness is not None:
            base_params['stiffness'] = cp.stiffness
        if cp.damping is not None:
            base_params['damping'] = cp.damping
        if cp.friction_static is not None:
            base_params['friction_static'] = cp.friction_static
        if cp.friction_dynamic is not None:
            base_params['friction_dynamic'] = cp.friction_dynamic
        if cp.max_penetration is not None:
            base_params['max_penetration'] = cp.max_penetration

        return base_params
