
class StateVector:
    def __init__(self):
        X = Y = Z = 0.0          # Position in inertial frame (m)
        u = v = w = 0.0          # Velocity in body frame (m/s)
        qw = qx = qy = qz = 0.0  # Orientation (quaternion)
        p = q = r = 0.0          # Angular rates in body frame (rad/s)
        self.state = [X, Y, Z, u, v, w, qw, qx, qy, qz, p, q, r]


    # Total airspeed: V = √(u² + v² + w²)
    def TotalAirspeed(self):
        u = self.state[3]
        v = self.state[4]
        w = self.state[5]
        return (u**2 + v**2 + w**2)**0.5
    

    # Angle of attack: α = atan2(w, u)
    def AngleOfAttack(self):
        u = self.state[3]
        w = self.state[5]
        from math import atan2
        return atan2(w, u)
    
    #Sideslip angle: β = asin(v / V)
    def SideslipAngle(self):
        u = self.state[3]
        v = self.state[4]
        w = self.state[5]
        V = (u**2 + v**2 + w**2)**0.5
        from math import asin
        if V == 0:
            return 0.0
        return asin(v / V)
