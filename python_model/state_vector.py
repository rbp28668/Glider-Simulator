from quaternion import Quaternion, quaternion_normalize
from v3d import V3d

class StateVector:
    """
    12 degree of freedom state vector for glider model.
    
    Earth Frame (Inertial Frame)
        Origin: Arbitrary reference point (typically runway threshold)
        X-axis: North (or arbitrary reference direction)
        Y-axis: East (perpendicular to X)
        Z-axis: Down (NED convention)

    Body Frame
        Origin: Aircraft center of gravity
        X-axis: Forward through nose
        Y-axis: Right wing direction (starboard)
        Z-axis: Down through belly (perpendicular to XY plane)

    Following aerospace standard (NED - North-East-Down):
        Altitude: Negative Z values (flying at 1000m → Z = -1000)
        Roll right: Positive roll angle/rate
        Pitch up: Positive pitch angle/rate
        Yaw right: Positive yaw angle/rate

    State Vector Elements:
        Position (Earth Frame) - X, Y, Z [meters]
        X: Position along Earth's North axis
        Y: Position along Earth's East axis
        Z: Position along Earth's Down axis (altitude = -Z)

        Velocity (Body Frame) - u, v, w [m/s]
        u: Velocity along body X-axis (forward)
        v: Velocity along body Y-axis (right)
        w: Velocity along body Z-axis (down)

        Orientation (Quaternion) - qw, qx, qy, qz [dimensionless]
        The rotation needed to transform from Earth frame to body frame.
        qw: Scalar (real) part
        qx: Vector component about body X-axis
        qy: Vector component about body Y-axis
        qz: Vector component about body Z-axis
        Constraint: qw² + qx² + qy² + qz² = 1 (unit quaternion)

        Angular Velocity (Body Frame) - 3 States p, q, r [rad/s]
        p: Roll rate about body X-axis
        q: Pitch rate about body Y-axis
        r: Yaw rate about body Z-axis


    """
    def __init__(self):
        X = Y = Z = 0.0          # Position in inertial (world) frame (m)
        u = v = w = 0.0          # Velocity in body frame (m/s)
        qw = 1.0
        qx = qy = qz = 0.0       # Orientation (quaternion)
        p = q = r = 0.0          # Angular rates in body frame (rad/s)
        self.state = [X, Y, Z, u, v, w, qw, qx, qy, qz, p, q, r]

    # Accessor methods

    # Indexing to allow state_vector[i] access
    def __getitem__(self, index: int) -> float:
        return self.state[index]

    # Position: X, Y, Z in world frame
    def position(self) -> V3d:
        return self.state[0], self.state[1], self.state[2]
    
    # X, Y, Z setter
    def set_position(self, p:V3d) -> None:
        self.state[0] = p[0]
        self.state[1] = p[1]
        self.state[2] = p[2]

    # Velocity: u, v, w in body frame
    def velocity(self) -> V3d:
        return self.state[3], self.state[4], self.state[5]
    
    def set_velocity(self, v:V3d) -> None:
        self.state[3] = v[0]
        self.state[4] = v[1]
        self.state[5] = v[2]

    # Orientation: qw, qx, qy, qz in quaternion form
    def orientation(self) -> Quaternion:
        return self.state[6], self.state[7], self.state[8], self.state[9]
    
    def set_orientation(self, q:Quaternion) -> None:
        self.state[6] = q[0]
        self.state[7] = q[1]
        self.state[8] = q[2]
        self.state[9] = q[3]
    
    # Angular velocity: p, q, r in body frame around c.g.
    def angular_velocity(self) -> V3d:
        return self.state[10], self.state[11], self.state[12]
    
    def set_angular_velocity(self, av: V3d) -> None:
        self.state[10] = av[0]
        self.state[11] = av[1]
        self.state[12] = av[2]


    def copy(self) -> StateVector:
        new_sv = StateVector()
        new_sv.state = self.state.copy()
        return new_sv

    # Create a new StateVector offset by a fraction of another StateVector
    # For RK4 integration steps
    def offset(self, deltaState: StateVector, fraction: float) -> StateVector:
        new_sv = StateVector()
        new_sv.state = [self.state[i] + deltaState.state[i] * fraction for i in range(13)]
        return new_sv
    
    def rk4_sum(self, k1: StateVector, k2: StateVector, k3: StateVector, k4: StateVector, dt: float) -> StateVector:
        # Combine RK4 increments to produce new state
        # was:       new_state = [ state[i] + (dt/6)*(k1[i] + 2*k2[i] + 2*k3[i] + k4[i])  for i in range(13)  ]
        new_sv = StateVector()
        new_sv.state = [
            self.state[i] + (dt/6)*(k1.state[i] + 2*k2.state[i] + 2*k3.state[i] + k4.state[i])
            for i in range(13)
        ]
        return new_sv

    def normalize_orientation(self) -> None:
        quat = self.orientation()
        norm = quaternion_normalize(quat)
        self.set_orientation(norm)

    # Total airspeed: V = √(u² + v² + w²)
    def TotalAirspeed(self) -> float:
        u = self.state[3]
        v = self.state[4]
        w = self.state[5]
        return (u**2 + v**2 + w**2)**0.5
    

    # Angle of attack: α = atan2(w, u)
    def AngleOfAttack(self) -> float:
        u = self.state[3]
        w = self.state[5]
        from math import atan2
        return atan2(w, u)
    
    #Sideslip angle: β = asin(v / V)
    def SideslipAngle(self) -> float:
        u = self.state[3]
        v = self.state[4]
        w = self.state[5]
        V = (u**2 + v**2 + w**2)**0.5
        from math import asin
        if V == 0:
            return 0.0
        return asin(v / V)


    
