
#pragma once

#include <cmath>
#include <algorithm>

#include "sim_types.h"
#include "ask21.h"
#include "state_vector.h"
#include "control_inputs.h"
#include "world.h"
#include "v3d.h"

//   This is the aerodynamic model for the simulation.
class Model {

	// Minimum airspeed for aerodynamic calculations (m/s)
	NumberT MIN_AIRSPEED = 0.1f;

	// Maximum total force/moment to prevent numerical overflow
	NumberT MAX_FORCE = 100000.0f;
	NumberT MAX_MOMENT = 500000.0f;

	inline NumberT clamp(NumberT value, NumberT min_val, NumberT max_val) {
		// Clamp value to range [min_val, max_val].
		return std::max(min_val, std::min(max_val, value));
	}

	inline NumberT safe_value(NumberT value, NumberT dflt = 0.0) {
		// Return default if value is NaN or Inf.
		return isnan(value) || isinf(value) ? dflt : value;
	}


public:

	// Note - these are public for testing
	void wing_forces(const StateVector<NumberT>& state, const ASK21& aircraft, const ControlInputs& control_inputs, const World& world, const V3d<NumberT>& relative_velocity,
		V3d<NumberT>& forces, V3d<NumberT>& moments);

	void tailplane_forces(const StateVector<NumberT>& state, const ASK21& aircraft, const ControlInputs& control_inputs, const World& world, const V3d<NumberT>& relative_velocity,
		V3d<NumberT>& forces, V3d<NumberT>& moments);

	void fin_forces(const StateVector<NumberT>& state, const ASK21& aircraft, const ControlInputs& control_inputs, const World& world, const V3d<NumberT>& relative_velocity,
		V3d<NumberT>& forces, V3d<NumberT>& moments);

	void fuselage_forces(const StateVector<NumberT>& state, const ASK21& aircraft, const ControlInputs& control_inputs, const World& world, const V3d<NumberT>& relative_velocity,
		V3d<NumberT>& forces, V3d<NumberT>& moments);



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
	void calculate_aerodynamics(const StateVector<NumberT>& state, const ASK21& aircraft, const ControlInputs& control_inputs, const World& world, const V3d<NumberT>& relative_velocity,
		V3d<NumberT>& forces, V3d<NumberT>& moments);

};