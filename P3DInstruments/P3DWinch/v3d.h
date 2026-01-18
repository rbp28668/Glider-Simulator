// type declaration for 3D vector
// Note - can be used for position, velocity, acceleration, angles, angular rates etc.

#ifndef V3D_H
#define V3D_H

#include <cmath>

template <typename T>
struct V3d
{

	T data[3];

	V3d()
	{
		data[0] = 0;
		data[1] = 0;
		data[2] = 0;
	}

	V3d(T x, T y, T z)
	{
		data[0] = x;
		data[1] = y;
		data[2] = z;
	}

	T& operator[](int idx)
	{
		return data[idx];
	}

	T operator[](int idx) const
	{
		return data[idx];
	}

	// Convenience functions for flight dynamics calculations

	// Total airspeed: V = √(u² + v² + w²)
	T TotalAirspeed(const V3d& velocity)
	{
		T u = velocity[0], v = velocity[1], w = velocity[2];
		return sqrt(u * u + v * v + w * w);
	}

	// Angle of attack: α = atan2(w, u)
	T AngleOfAttack(const V3d& velocity)
	{
		T u = velocity[0]; // x +ve forward
		T w = velocity[2]; // z +ve down
		return atan2(w, u);
	}

	// Sideslip angle: β = asin(v / V)
	T SideslipAngle(const V3d& velocity)
	{
		T u = velocity[0], v = velocity[1], w = velocity[2];
		auto V = sqrt(u * u + v * v + w * w);
		if (V < 0.001) // very slow
			return 0.0;
		return asin(v / V);
	}
};

#endif // V3D_H
