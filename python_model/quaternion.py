from math import sin, cos, atan2, asin, sqrt, copysign, pi

type Quaternion = tuple[float, float, float, float]  # (qw, qx, qy, qz)

# Quaternion Multiplication
def quaternion_multiply(q1: Quaternion, q2: Quaternion) -> Quaternion:
    """
    Multiply two quaternions: q = q1 ⊗ q2
    """
    w1, x1, y1, z1 = q1
    w2, x2, y2, z2 = q2
    
    w = w1*w2 - x1*x2 - y1*y2 - z1*z2
    x = w1*x2 + x1*w2 + y1*z2 - z1*y2
    y = w1*y2 - x1*z2 + y1*w2 + z1*x2
    z = w1*z2 + x1*y2 - y1*x2 + z1*w2
    
    return (w, x, y, z)

# Quaternion Conjugate
def quaternion_conjugate(q: Quaternion) -> Quaternion:
    """
    Conjugate of quaternion: q* = [qw, -qx, -qy, -qz]
    """
    return (q[0], -q[1], -q[2], -q[3])

# Quaternion Normalization
def quaternion_normalize(q: Quaternion) -> Quaternion:
    """
    Normalize quaternion to unit length
    """
    norm = sqrt(q[0]**2 + q[1]**2 + q[2]**2 + q[3]**2)
    if norm == 0:
        return (1.0, 0.0, 0.0, 0.0)  # Default to no rotation
    return (q[0]/norm, q[1]/norm, q[2]/norm, q[3]/norm)

# Euler Angles to Quaternion
def euler_to_quaternion(phi: float, theta: float, psi: float) -> Quaternion:
    """
    Convert Euler angles (roll, pitch, yaw) to quaternion
    Convention: ZYX (yaw-pitch-roll)
    
    Args:
        phi: Roll angle (rad)
        theta: Pitch angle (rad)
        psi: Yaw angle (rad)
    
    Returns:
        [qw, qx, qy, qz]
    """
    cy = cos(psi * 0.5)
    sy = sin(psi * 0.5)
    cp = cos(theta * 0.5)
    sp = sin(theta * 0.5)
    cr = cos(phi * 0.5)
    sr = sin(phi * 0.5)
    
    qw = cr * cp * cy + sr * sp * sy
    qx = sr * cp * cy - cr * sp * sy
    qy = cr * sp * cy + sr * cp * sy
    qz = cr * cp * sy - sr * sp * cy
    
    return (qw, qx, qy, qz)


# Quaternion to Euler Angles
def quaternion_to_euler(qw: float, qx: float, qy: float, qz: float) -> tuple[float, float, float]:

    """
    Convert quaternion to Euler angles (roll, pitch, yaw)
    Convention: ZYX (yaw-pitch-roll)
    
    Returns:
        phi: Roll angle (rad)
        theta: Pitch angle (rad)
        psi: Yaw angle (rad)
    """

    # Roll (phi)
    sinr_cosp = 2 * (qw * qx + qy * qz)
    cosr_cosp = 1 - 2 * (qx * qx + qy * qy)
    phi = atan2(sinr_cosp, cosr_cosp)
    
    # Pitch (theta)
    sinp = 2 * (qw * qy - qz * qx)
    if abs(sinp) >= 1:
        theta = copysign(pi / 2, sinp)  # Use 90° if out of range
    else:
        theta = asin(sinp)
    
    # Yaw (psi)
    siny_cosp = 2 * (qw * qz + qx * qy)
    cosy_cosp = 1 - 2 * (qy * qy + qz * qz)
    psi = atan2(siny_cosp, cosy_cosp)
    
    return phi, theta, psi

# Quaternion to Rotation Matrix
#def quaternion_to_rotation_matrix(qw, qx, qy, qz):

    # """
    # Convert quaternion to 3x3 rotation matrix
    # Transforms vectors from body frame to Earth frame
    # """
    # R = np.array([
    #     [1 - 2*(qy**2 + qz**2),  2*(qx*qy - qw*qz),      2*(qx*qz + qw*qy)],
    #     [2*(qx*qy + qw*qz),      1 - 2*(qx**2 + qz**2),  2*(qy*qz - qw*qx)],
    #     [2*(qx*qz - qw*qy),      2*(qy*qz + qw*qx),      1 - 2*(qx**2 + qy**2)]
    # ])

    # return R

# Vector Rotation
def quaternion_rotate_vector(q: Quaternion, v: tuple[float, float, float]) -> tuple[float, float, float]:

    """
    Rotate vector from body frame to Earth frame using quaternion
    v_earth = q ⊗ v ⊗ q*
    
    Args:
        q: Quaternion [qw, qx, qy, qz]
        v: Vector [vx, vy, vz]
    
    Returns:
        Rotated vector [vx', vy', vz']
    """

    # Convert vector to quaternion form [0, vx, vy, vz]
    v_quat = 0, v[0], v[1], v[2]
    
    # Compute q ⊗ v ⊗ q*
    q_conj = quaternion_conjugate(q)
    temp = quaternion_multiply(q, v_quat)
    result = quaternion_multiply(temp, q_conj)
    
    return result[1], result[2], result[3]


def quaternion_rotate_vector_inverse(q: Quaternion, v: tuple[float, float, float]) -> tuple[float, float, float]:

    """
    Rotate vector from Earth frame to body frame
    v_body = q* ⊗ v ⊗ q
    """
    q_conj = quaternion_conjugate(q)
    return quaternion_rotate_vector(q_conj, v)

# Quaternion Derivative
# The time derivative of a quaternion based on angular velocity:
def quaternion_derivative(q: Quaternion, omega: tuple[float, float, float]) -> Quaternion:
    """
    Calculate quaternion time derivative
    q̇ = 0.5 * q ⊗ ω

    Args:
        q: Quaternion [qw, qx, qy, qz]
        omega: Angular velocity [p, q, r] in body frame (rad/s)
               p = roll rate (about body X)
               q = pitch rate (about body Y)
               r = yaw rate (about body Z)

    Returns:
        Quaternion derivative [q̇w, q̇x, q̇y, q̇z]
    """
    qw, qx, qy, qz = q
    p, q_rate, r = omega  # roll, pitch, yaw rates

    q_dot_w = -0.5 * (qx*p + qy*q_rate + qz*r)
    q_dot_x =  0.5 * (qw*p + qy*r - qz*q_rate)
    q_dot_y =  0.5 * (qw*q_rate + qz*p - qx*r)
    q_dot_z =  0.5 * (qw*r + qx*q_rate - qy*p)

    return q_dot_w, q_dot_x, q_dot_y, q_dot_z

