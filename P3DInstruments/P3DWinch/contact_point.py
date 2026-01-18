#--- class contact_point.py
class ContactPoint:
    '''
    Class representing a contact point in 3D space for use in ground detection.

    Position is in body frame relative to datum (root leading edge on fuselage centerline):
    - x: positive forward
    - y: positive right (starboard)
    - z: positive down

    Contact types:
    - 'wheel': Landing gear wheel (rolling resistance longitudinally, full friction laterally)
    - 'skid': Tail skid or similar (full friction both directions)
    - 'wingtip': Wing tip contact (softer, full friction)
    '''
    def __init__(self, x: float, y: float, z: float,
                 contact_type: str = 'wheel',
                 stiffness: float = None,
                 damping: float = None,
                 friction_static: float = None,
                 friction_dynamic: float = None,
                 max_penetration: float = None):
        # Position relative to datum in body frame
        self.x = x
        self.y = y
        self.z = z

        # Contact type determines default parameters
        self.contact_type = contact_type

        # Optional parameter overrides (None means use defaults for contact_type)
        self.stiffness = stiffness
        self.damping = damping
        self.friction_static = friction_static
        self.friction_dynamic = friction_dynamic
        self.max_penetration = max_penetration

    def position_body(self) -> tuple[float, float, float]:
        """Return position as tuple for vector operations."""
        return (self.x, self.y, self.z)