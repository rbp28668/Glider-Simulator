#--- class contact_point.py
class ContactPoint:
    '''
    Class representing a contact point in 3D space for use in ground detection.
    '''
    def __init__(self, x: float, y: float, z: float):
        self.x = x
        self.y = y
        self.z = z