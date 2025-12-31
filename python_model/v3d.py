# type declaration for 3D vector

type V3d = tuple[float, float, float]  # (x,y,z) etc.


# Convenience functions for flight dynamics calculations

# Total airspeed: V = √(u² + v² + w²)
def TotalAirspeed(velocity: V3d) -> float:
    u = velocity[0]
    v = velocity[1]
    w = velocity[2]
    return (u**2 + v**2 + w**2)**0.5

# Angle of attack: α = atan2(w, u)
def AngleOfAttack(velocity: V3d) -> float:
    u = velocity[0]
    w = velocity[2]
    from math import atan2
    return atan2(w, u)

#Sideslip angle: β = asin(v / V)
def SideslipAngle(velocity: V3d) -> float:
    u = velocity[0]
    v = velocity[1]
    w = velocity[2]
    V = (u**2 + v**2 + w**2)**0.5
    from math import asin
    if V == 0:
        return 0.0
    return asin(v / V)