from v3d import V3d

class World :
    """
    Represents the environmental conditions of the simulation world.
    """
    def __init__(self):
        self.gravity = 9.81  # m/s² 
        self.air_density = 1.225  # kg/m³ at sea level
        self.wind_speed = 0.0  # m/s
        self.wind_direction = 0.0  # degrees from north

    def set_wind(self, speed: float, direction: float):
        """
        Set the wind conditions in the world.
        
        Args:
            speed: Wind speed in m/s
            direction: Wind direction in degrees from north
        """
        self.wind_speed = speed
        self.wind_direction = direction

    def get_wind_vector(self, position : V3d, t : float) -> V3d:
        """
        Get the wind vector in Cartesian coordinates.
        
        Args:
            position: A tuple (x, y, z) representing the position in meters.
            t: Current time in seconds.
        Returns:
            A tuple (wx, wy, wz) representing the wind vector components in m/s.
        """
        from math import radians, cos, sin
        dir_rad = radians(self.wind_direction)
        wx = self.wind_speed * cos(dir_rad)
        wy = self.wind_speed * sin(dir_rad)
        return (wx, wy, 0.0)  # Assuming no vertical wind component
    

    def get_ground_height(self, x: float, y: float) -> float:
        """
        Get the ground height at a given (x, y) position.
        
        Args:
            x: X coordinate in meters
            y: Y coordinate in meters
        
        Returns:
            Ground height (Z) in meters
        """
        return 0.0  # Flat ground at Z=0 for simplicity