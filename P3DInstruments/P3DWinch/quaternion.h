#ifndef QUATERNION_H
#define QUATERNION_H
#include <cmath>
#include "v3d.h"

template <typename T>
struct Quaternion
{
    const double pi = 3.14159265358979323846;

    T data[4]; // (qw, qx, qy, qz)

    Quaternion()
    {
        data[0] = 1; // w
        data[1] = 0; // x
        data[2] = 0; // y
        data[3] = 0; // z
    }

    Quaternion(T w, T x, T y, T z)
    {
        data[0] = w;
        data[1] = x;
        data[2] = y;
        data[3] = z;
    }

    Quaternion(const Quaternion& q) {
        data[0] = q.data[0];
        data[1] = q.data[1];
        data[2] = q.data[2];
        data[3] = q.data[3];
    }

    Quaternion& operator=(const Quaternion& other) {
        if (this == &other) return *this;

        data[0] = other.data[0];
        data[1] = other.data[1];
        data[2] = other.data[2];
        data[3] = other.data[3];
        return *this;
    }


    T &operator[](int idx)
    {
        return data[idx];
    }

    T operator[](int idx) const
    {
        return data[idx];
    }

    // Quaternion Multiplication
    // Multiply two quaternions: q = q1 ⊗ q2
    Quaternion<T> operator*(const Quaternion<T> &q2) const
    {
        T w1 = data[0], x1 = data[1], y1 = data[2], z1 = data[3];
        T w2 = q2[0], x2 = q2[1], y2 = q2[2], z2 = q2[3];

        T w = w1 * w2 - x1 * x2 - y1 * y2 - z1 * z2;
        T x = w1 * x2 + x1 * w2 + y1 * z2 - z1 * y2;
        T y = w1 * y2 - x1 * z2 + y1 * w2 + z1 * x2;
        T z = w1 * z2 + x1 * y2 - y1 * x2 + z1 * w2;
        return Quaternion(w, x, y, z);
    }

    // Quaternion Conjugate
    // Conjugate of quaternion: q* = [qw, -qx, -qy, -qz]
    Quaternion<T> conjugate()
    {
        return Quaternion(data[0], -data[1], -data[2], -data[3]);
    }

    // Quaternion Normalization
    // Normalize quaternion to unit length
    // Note- in place normalization
    void normalize()
    {
        T w = data[0], x = data[1], y = data[2], z = data[3];
        T norm = sqrt(w * w + x * x + y * y + z * z);
        if (norm < 0.0001f) {                         // very small rather than 0
            data[0] = 1.0f;
            data[1] = 0.0f;
            data[2] = 0.0f;
            data[3] = 0.0f;
        }
        else {
            data[0] = w / norm;
            data[1] = x / norm;
            data[2] = y / norm;
            data[3] = z / norm;
        }
    }

    // Euler Angles to Quaternion
    // Convert Euler angles (roll, pitch, yaw) to quaternion
    // Convention: ZYX (yaw-pitch-roll)
    // Args:
    //     phi: Roll angle (rad)
    //     theta: Pitch angle (rad)
    //     psi: Yaw angle (rad)
    // Returns:
    //     [qw, qx, qy, qz]
    static Quaternion from_euler_angles(T phi, T theta, T psi)
    {
        T cy = cos(psi * 0.5);
        T sy = sin(psi * 0.5);
        T cp = cos(theta * 0.5);
        T sp = sin(theta * 0.5);
        T cr = cos(phi * 0.5);
        T sr = sin(phi * 0.5);

        T qw = cr * cp * cy + sr * sp * sy;
        T qx = sr * cp * cy - cr * sp * sy;
        T qy = cr * sp * cy + sr * cp * sy;
        T qz = cr * cp * sy - sr * sp * cy;

        return Quaternion(qw, qx, qy, qz);
    }

    // Quaternion to Euler Angles
    // Convert quaternion to Euler angles (roll, pitch, yaw)
    // Convention: ZYX (yaw-pitch-roll)
    // Returns:
    //     phi: Roll angle (rad)
    //     theta: Pitch angle (rad)
    //     psi: Yaw angle (rad)
    V3d<T> to_euler() const
    {
        T qw = data[0], qx = data[1], qy = data[2], qz = data[3];
        // Roll (phi)
        T sinr_cosp = 2 * (qw * qx + qy * qz);
        T cosr_cosp = 1 - 2 * (qx * qx + qy * qy);
        T phi = atan2(sinr_cosp, cosr_cosp);

        // Pitch (theta)
        T sinp = 2 * (qw * qy - qz * qx);
        T theta = 0;
        if (abs(sinp) >= 1)
            theta = copysign((T)pi / 2, sinp); // Use 90° if out of range
        else
            theta = asin(sinp);

        // Yaw (psi)
        T siny_cosp = 2 * (qw * qz + qx * qy);

        T cosy_cosp = 1 - 2 * (qy * qy + qz * qz);
        T psi = atan2(siny_cosp, cosy_cosp);

        return V3d<T>(phi, theta, psi);
    }

    // Vector Rotation
    // Rotate vector from body frame to Earth frame using quaternion
    // v_earth = q ⊗ v ⊗ q*
    // Args:
    //     q: Quaternion [qw, qx, qy, qz]
    //     v: Vector [vx, vy, vz]
    // Returns:
    //     Rotated vector [vx', vy', vz']
    V3d<T> rotate_vector(const V3d<T> &v)
    {

        // Convert vector to quaternion form [0, vx, vy, vz]
        Quaternion v_quat = Quaternion(0, v[0], v[1], v[2]);

        // Compute q ⊗ v ⊗ q*
        Quaternion q_conj = conjugate();
        Quaternion temp = (*this) * v_quat; // quaternion_multiply(q, v_quat)
        Quaternion result = temp * q_conj;  // quaternion_multiply(temp, q_conj)

        return V3d<T>(result[1], result[2], result[3]);
    }

    //    Rotate vector from Earth frame to body frame
    //    v_body = q* ⊗ v ⊗ q
    V3d<T> rotate_vector_inverse(const V3d<T> &v)
    {
        Quaternion q_conj = this->conjugate();
        return q_conj.rotate_vector(v);
    }

    // Quaternion Derivative
    // The time derivative of a quaternion based on angular velocity:
    // Calculate quaternion time derivative
    // q̇ = 0.5 * q ⊗ ω
    // Args:
    //     q: Quaternion [qw, qx, qy, qz]
    //     omega: Angular velocity [p, q, r] in body frame (rad/s)
    //            p = roll rate (about body X)
    //            q = pitch rate (about body Y)
    //            r = yaw rate (about body Z)
    // Returns:
    //     Quaternion derivative [q̇w, q̇x, q̇y, q̇z]
    Quaternion<T> derivative(const V3d<T> &omega)
    {
        T qw = data[0], qx = data[1], qy = data[2], qz = data[3];
        T p = omega[0], q_rate = omega[1], r = omega[2]; // roll, pitch, yaw rates

        T q_dot_w = -0.5f * (qx * p + qy * q_rate + qz * r);
        T q_dot_x = 0.5f * (qw * p + qy * r - qz * q_rate);
        T q_dot_y = 0.5f * (qw * q_rate + qz * p - qx * r);
        T q_dot_z = 0.5f * (qw * r + qx * q_rate - qy * p);

        return Quaternion(q_dot_w, q_dot_x, q_dot_y, q_dot_z);
    }
};

#endif // QUATERNION_H