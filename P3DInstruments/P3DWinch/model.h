
#pragma once

#include <cmath>
#include <algorithm>

#include "ask21.h"
#include "state_vector.h"
#include "control_inputs.h"
#include "world.h"
#include "v3d.h"

//   This is the aerodynamic model for the simulation.
class Model {

	// Minimum airspeed for aerodynamic calculations (m/s)
	float MIN_AIRSPEED = 0.1f;

	// Maximum total force/moment to prevent numerical overflow
	float MAX_FORCE = 100000.0f;
	float MAX_MOMENT = 500000.0f;

	inline float clamp(float value, float min_val, float max_val) {
		// Clamp value to range [min_val, max_val].
		return std::max(min_val, std::min(max_val, value));
	}

	inline float safe_value(float value, float dflt = 0.0) {
		// Return default if value is NaN or Inf.
		return isnan(value) || isinf(value) ? dflt : value;
	}


public:

	// Note - these are public for testing
	void wing_forces(const StateVector<float>& state, const ASK21& aircraft, const ControlInputs& control_inputs, const World& world, const V3d<float>& relative_velocity,
		V3d<float>& forces, V3d<float>& moments);

	void tailplane_forces(const StateVector<float>& state, const ASK21& aircraft, const ControlInputs& control_inputs, const World& world, const V3d<float>& relative_velocity,
		V3d<float>& forces, V3d<float>& moments);

	void fin_forces(const StateVector<float>& state, const ASK21& aircraft, const ControlInputs& control_inputs, const World& world, const V3d<float>& relative_velocity,
		V3d<float>& forces, V3d<float>& moments);

	void fuselage_forces(const StateVector<float>& state, const ASK21& aircraft, const ControlInputs& control_inputs, const World& world, const V3d<float>& relative_velocity,
		V3d<float>& forces, V3d<float>& moments);



	Model() {}
	~Model() {}


	// Calculate aerodynamic forces and moments
	// Args:
	//     state: Current state vector
	//     aircraft: The aircraft model
	//     control_inputs: Current control surface deflections
	//     world: The simulation world
	//     relative_velocity: The aircraft velocity relative to the air-mass (in body axes, wind corrected)
	// Returns:
	//     forces_body: [Fx, Fy, Fz] (N)
	//     moments_body: [L, M, N] (N·m)
	void calculate_aerodynamics(const StateVector<float>& state, const ASK21& aircraft, const ControlInputs& control_inputs, const World& world, const V3d<float>& relative_velocity,
		V3d<float>& forces, V3d<float>& moments);

};