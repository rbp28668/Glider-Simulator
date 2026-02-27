#pragma once
#include <vector>
#include <algorithm>
#include "state_vector.h"
#include "contact_point.h"
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
    float penetration = 0.0;  // m (positive = into ground)
    float normal_force = 0.0; // N (positive = pushing up)
    float friction_force_long = 0.0;
    float friction_force_lat = 0.0; // (longitudinal, lateral) in N
    V3d<float> force_body;          // Force in body frame
    V3d<float> moment_body;         // Moment about CG in body frame
    V3d<float> position_earth;      // Contact position in earth frame
};

// Ground contact physics calculator.
// Implements spring-damper normal forces and Coulomb friction.
class GroundContact
{

    // Velocity threshold for static/dynamic friction transition
    float FRICTION_VELOCITY_THRESHOLD = 0.1f; // m/s

    float ROLLING_RESISTANCE = 0.03f; // Coefficient of rolling resistance for wheels

    // Maximum force per contact point (numerical stability)
    float MAX_CONTACT_FORCE = 100000.0f; // N

public:

    GroundContact();

    // Calculate total ground contact forces and moments.
    void calculate_ground_forces(const StateVector<float>& state, std::vector<const ContactPoint*>& contact_points, const World& world, float cg_offset,
        V3d<float>& forces_body, V3d<float>& moments_body,
        std::vector<ContactResult>& results);

    // Calculate forces for a single contact point.
    ContactResult _calculate_single_contact(const StateVector<float>& state, const ContactPoint& cp, const World& world, float cg_offset);

    // Calculate velocity of contact point in body frame.
     V3d<float> _get_contact_velocity(const StateVector<float>& state, const ContactPoint& cp, float cg_offset);

    // Calculate friction forces using Coulomb model with velocity blending.
     void _calculate_friction(V3d<float> vel_earth, float normal_force, const ContactPoint& cp, float& F_x, float& F_y);
};