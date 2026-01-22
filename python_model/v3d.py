# type declaration for 3D vector

type V3d = tuple[float, float, float]  # (x,y,z) etc.


# Convenience functions for flight dynamics calculations

# Total airspeed: V = √(u² + v² + w²)
# With overflow protection
MAX_VELOCITY = 500.0  # m/s - well beyond any realistic flight speed

def TotalAirspeed(velocity: V3d) -> float:
    u = min(max(velocity[0], -MAX_VELOCITY), MAX_VELOCITY)
    v = min(max(velocity[1], -MAX_VELOCITY), MAX_VELOCITY)
    w = min(max(velocity[2], -MAX_VELOCITY), MAX_VELOCITY)
    return (u**2 + v**2 + w**2)**0.5

# Angle of attack: α = atan2(w, u)
def AngleOfAttack(velocity: V3d) -> float:
    u = velocity[0] # x +ve forward
    w = velocity[2] # z +ve down
    from math import atan2
    return atan2(w, u)

#Sideslip angle: β = asin(v / V)
def SideslipAngle(velocity: V3d) -> float:
    u = min(max(velocity[0], -MAX_VELOCITY), MAX_VELOCITY)
    v = min(max(velocity[1], -MAX_VELOCITY), MAX_VELOCITY)
    w = min(max(velocity[2], -MAX_VELOCITY), MAX_VELOCITY)
    V = (u**2 + v**2 + w**2)**0.5
    from math import asin
    if V < 0.001 : # very slow
        return 0.0
    return asin(max(-1.0, min(1.0, v / V)))