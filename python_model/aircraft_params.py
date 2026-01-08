from math import radians


class AircraftParameters:

    def __init__(self) -> None:
        self.CG = 0.0                           # CG from datum
        self.AR = 20.0                          # Aspect ratio
        self.oswald = 1.0                       # oswald factor i.e. how close to perfect elliptical wing
        self.dihedral_angle = radians(4.0)      # dihedral angle under each wing.