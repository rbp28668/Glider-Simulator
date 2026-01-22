#pragma once
#include <array>
#include "quaternion.h"

/*
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
        u: Velocity along body X-axis (+ve: forward)
        v: Velocity along body Y-axis (+ve: right)
        w: Velocity along body Z-axis (+ve: down)

        Orientation (Quaternion) - qw, qx, qy, qz [dimensionless]
        The rotation needed to transform from Earth frame to body frame.
        qw: Scalar (real) part
        qx: Vector component about body X-axis
        qy: Vector component about body Y-axis
        qz: Vector component about body Z-axis
        Constraint: qw² + qx² + qy² + qz² = 1 (unit quaternion)

        Angular Velocity (Body Frame) - 3 States p, q, r [rad/s]
        p: Roll rate about body X-axis   +ve is rolling right
        q: Pitch rate about body Y-axis  +ve is pitching up
        r: Yaw rate about body Z-axis    +ve is yawing right
*/

template <typename T>
class StateVector
{

    V3d<T> _position;           // X, Y, Z in world frame
    V3d<T> _velocity;           // u, v, w in body frame
    Quaternion<T> _orientation; // qw, qx, qy, qz
    V3d<T> _angular_velocity;   // p, q, r in body frame

    const double pi = 3.14159265358979323846;


public:
    // Constructors

    // Default constructor - stationary at origin, no rotation
    StateVector()
    {
    }

    // Parameterized constructor from given components
    StateVector(const V3d<T> &position,
                const V3d<T> &velocity,
                const Quaternion<T> &orientation,
                const V3d<T> &angular_velocity)
        : _position(position),
          _velocity(velocity),
          _orientation(orientation),
          _angular_velocity(angular_velocity)
    {
    }

    // Constructor from array of 13 elements - useful for RK4 steps & serialisation
    StateVector(const std::array<T, 13> &state_array)
        : _position(state_array[0], state_array[1], state_array[2]),
        _velocity(state_array[3], state_array[4], state_array[5]),
        _orientation(state_array[6], state_array[7], state_array[8], state_array[9]),
        _angular_velocity(state_array[10], state_array[11], state_array[12])
    {
    }

    // Copy constructor
    StateVector(const StateVector &other)
        : _position(other._position),
          _velocity(other._velocity),
          _orientation(other._orientation),
          _angular_velocity(other._angular_velocity)
    {
    }

    // As we have a copy constructor also need assignment operator otherwise
    // we get attempting to reference a deleted function
    StateVector& operator=(const StateVector& other)
    {
        // Guard self assignment
        if (this == &other)
            return *this;

        _position = other._position;
        _velocity = other._velocity;
        _orientation = other._orientation;
        _angular_velocity = other._angular_velocity;
        return *this;
    }

    // Accessor methods

    V3d<T> position() const
    {
        return _position;
    }

    void set_position(const V3d<T> &position)
    {
        this->_position = position;
    }

    // Velocity: u, v, w in body frame
    V3d<T> velocity() const
    {
        return _velocity;
    }

    void set_velocity(const V3d<T> &velocity)
    {
        this->_velocity = velocity;
    }

    // Orientation: qw, qx, qy, qz in quaternion form
    Quaternion<T> orientation() const
    {
        return _orientation;
    }

    void set_orientation(const Quaternion<T> &q)
    {
        this->_orientation = q;
    }

    // Angular velocity: p, q, r in body frame around c.g.
    V3d<T> angular_velocity() const
    {
        return _angular_velocity;
    }

    void set_angular_velocity(const V3d<T> &av)
    {
        this->_angular_velocity = av;
    }

    StateVector<T> copy()
    {
        return StateVector<T>(*this);
    }

    // Create a new StateVector offset by a fraction of another StateVector
    // For RK4 integration steps
    StateVector<T> offset(const StateVector<T> &deltaState, T fraction) const
    {

        std::array<T, 13> state_array;
        state_array[0] = _position[0] + deltaState._position[0] * fraction;
        state_array[1] = _position[1] + deltaState._position[1] * fraction;
        state_array[2] = _position[2] + deltaState._position[2] * fraction;
        state_array[3] = _velocity[0] + deltaState._velocity[0] * fraction;
        state_array[4] = _velocity[1] + deltaState._velocity[1] * fraction;
        state_array[5] = _velocity[2] + deltaState._velocity[2] * fraction;
        state_array[6] = _orientation[0] + deltaState._orientation[0] * fraction;
        state_array[7] = _orientation[1] + deltaState._orientation[1] * fraction;
        state_array[8] = _orientation[2] + deltaState._orientation[2] * fraction;
        state_array[9] = _orientation[3] + deltaState._orientation[3] * fraction;
        state_array[10] = _angular_velocity[0] + deltaState._angular_velocity[0] * fraction;
        state_array[11] = _angular_velocity[1] + deltaState._angular_velocity[1] * fraction;
        state_array[12] = _angular_velocity[2] + deltaState._angular_velocity[2] * fraction;

        return StateVector<T>(state_array);
    }

    StateVector<T> rk4_sum(const StateVector &k1, const StateVector &k2, const StateVector &k3, const StateVector &k4, T dt) const
    {
        // Combine RK4 increments to produce new state
        // was:       new_state = [ state[i] + (dt/6)*(k1[i] + 2*k2[i] + 2*k3[i] + k4[i])  for i in range(13)  ]

        std::array<T, 13> state_array;
        state_array[0] = _position[0] + (dt / 6) * (k1._position[0] + 2 * k2._position[0] + 2 * k3._position[0] + k4._position[0]);
        state_array[1] = _position[1] + (dt / 6) * (k1._position[1] + 2 * k2._position[1] + 2 * k3._position[1] + k4._position[1]);
        state_array[2] = _position[2] + (dt / 6) * (k1._position[2] + 2 * k2._position[2] + 2 * k3._position[2] + k4._position[2]);
        state_array[3] = _velocity[0] + (dt / 6) * (k1._velocity[0] + 2 * k2._velocity[0] + 2 * k3._velocity[0] + k4._velocity[0]);
        state_array[4] = _velocity[1] + (dt / 6) * (k1._velocity[1] + 2 * k2._velocity[1] + 2 * k3._velocity[1] + k4._velocity[1]);
        state_array[5] = _velocity[2] + (dt / 6) * (k1._velocity[2] + 2 * k2._velocity[2] + 2 * k3._velocity[2] + k4._velocity[2]);
        state_array[6] = _orientation[0] + (dt / 6) * (k1._orientation[0] + 2 * k2._orientation[0] + 2 * k3._orientation[0] + k4._orientation[0]);
        state_array[7] = _orientation[1] + (dt / 6) * (k1._orientation[1] + 2 * k2._orientation[1] + 2 * k3._orientation[1] + k4._orientation[1]);
        state_array[8] = _orientation[2] + (dt / 6) * (k1._orientation[2] + 2 * k2._orientation[2] + 2 * k3._orientation[2] + k4._orientation[2]);
        state_array[9] = _orientation[3] + (dt / 6) * (k1._orientation[3] + 2 * k2._orientation[3] + 2 * k3._orientation[3] + k4._orientation[3]);
        state_array[10] = _angular_velocity[0] + (dt / 6) * (k1._angular_velocity[0] + 2 * k2._angular_velocity[0] + 2 * k3._angular_velocity[0] + k4._angular_velocity[0]);
        state_array[11] = _angular_velocity[1] + (dt / 6) * (k1._angular_velocity[1] + 2 * k2._angular_velocity[1] + 2 * k3._angular_velocity[1] + k4._angular_velocity[1]);
        state_array[12] = _angular_velocity[2] + (dt / 6) * (k1._angular_velocity[2] + 2 * k2._angular_velocity[2] + 2 * k3._angular_velocity[2] + k4._angular_velocity[2]);

        return StateVector<T>(state_array);
    }

    void normalize_orientation()
    {
        _orientation.normalize();
    }

    // Total airspeed: V = √(u² + v² + w²)
    T TotalAirspeed() const
    {
        auto u = _velocity[0];
        auto v = _velocity[1];
        auto w = _velocity[2];
        return sqrt(u * u + v * v + w * w);
    }

    // Angle of attack: α = atan2(w, u)
    T AngleOfAttack() const
    {
        auto u = _velocity[0];
        auto w = _velocity[2];
        return atan2(w, u);
    }

    // Sideslip angle: β = asin(v / V)
    T SideslipAngle() const
    {
        auto u = _velocity[0], v = _velocity[1], w = _velocity[2];
        T V = sqrt(u * u + v * v + w * w);

        if (V < 0.001) // zero if very slow
            return 0.0;
        return asin(v / V); // must be in range -1..1
    }

    T Altitude() const
    {
        // Altitude is negative Z in NED convention
        return -_position[2];
    }

    T Heading() const
    {
        // Yaw angle from quaternion
        V3d<T> o = _orientation.to_euler();
        T psi = o[2]; //
        T heading_deg = psi * 180 / pi;
        if (heading_deg < 0)
            heading_deg += 360;
        return heading_deg;
    }

    // Attitude: roll, pitch, heading in radians
    V3d<T> Attitude() const
    {
        return _orientation.to_euler();
    }

    T VerticalSpeed() const
    {
        // Vertical speed in m/s (negative w in NED convention)
        auto v = _orientation.rotate_vector(_velocity); // Convert body to earth frame
        return -v[2];
    }
};