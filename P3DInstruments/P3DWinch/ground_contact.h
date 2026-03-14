#pragma once
#include <vector>
#include <algorithm>
#include "state_vector.h"
#include "contact_point.h"
#include "control_inputs.h"
#include "world.h"
#include "v3d.h"

// Ground contact physics for glider simulation.

// Implements spring-damper normal forces and Coulomb friction model
// for wheel, skid, and wingtip contacts.

// Coordinate system notes:
// - Body frame: X forward, Y right, Z down
// - Earth frame: NED (X north, Y east, Z down)
// - Ground height is Z coordinate in earth frame (Z=0 is ground level)
// - Aircraft altitude is negative Z (flying at 1000m -> Z = -1000)

// Result of a single contact point calculation.
class ContactResult
{

public:
	bool in_contact = false;
	NumberT penetration = 0.0;  // m (positive = into ground)
	NumberT normal_force = 0.0; // N (positive = pushing up)
	NumberT friction_force_long = 0.0;
	NumberT friction_force_lat = 0.0; // (longitudinal, lateral) in N
	V3d<NumberT> force_body;          // Force in body frame
	V3d<NumberT> moment_body;         // Moment about CG in body frame
	V3d<NumberT> position_earth;      // Contact position in earth frame
};

// Ground contact physics calculator.
// Implements spring-damper normal forces and Coulomb friction.
class GroundContact
{

	// Velocity threshold for static/dynamic friction transition
	NumberT FRICTION_VELOCITY_THRESHOLD = 0.1f; // m/s

	NumberT ROLLING_RESISTANCE = 0.03f; // Coefficient of rolling resistance for wheels

	// Maximum force per contact point (numerical stability)
	NumberT MAX_CONTACT_FORCE = 100000.0f; // 

	bool on_ground = false;

public:

	GroundContact();

	// Calculate total ground contact forces and moments.
	void calculate_ground_forces(const StateVector<NumberT>& state, std::vector<const ContactPoint*>& contact_points, const World& world, NumberT cg_offset, float brake,
		V3d<NumberT>& forces_body, V3d<NumberT>& moments_body,
		std::vector<ContactResult>& results);

	// Calculate forces for a single contact point.
	ContactResult _calculate_single_contact(const StateVector<NumberT>& state, const ContactPoint& cp, const World& world, NumberT cg_offset, float brake);

	// Calculate velocity of contact point in body frame.
	V3d<NumberT> _get_contact_velocity(const StateVector<NumberT>& state, const ContactPoint& cp, NumberT cg_offset);

	// Calculate friction forces using Coulomb model with velocity blending.
	void _calculate_friction(V3d<NumberT> vel_earth, NumberT normal_force, const ContactPoint& cp, const V3d<NumberT>& heading_earth, float brake, NumberT& F_x, NumberT& F_y);

	bool is_on_ground() const;

};