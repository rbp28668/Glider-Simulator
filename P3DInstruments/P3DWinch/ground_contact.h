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
    // Args:
    // cg_position: CG x-coordinate relative to datum (negative = aft)
    GroundContact()
        
    {
    }

    // Calculate total ground contact forces and moments.

    // Args:
    //     state: Current aircraft state
    //     contact_points: List of ContactPoint objects
    //     world: World object for ground height lookup
    //     cg_offset: CG position relative to datum (negative = aft)
    // Returns:
    //     forces_body: Total force in body frame (Fx, Fy, Fz)
    //     moments_body: Total moment about CG in body frame (L, M, N)
    //     results: List of ContactResult for each contact point
    void calculate_ground_forces(const StateVector<float> &state, std::vector<const ContactPoint*> &contact_points, const World &world, float cg_offset,
                                 V3d<float> &forces_body, V3d<float> &moments_body,
                                 std::vector<ContactResult> &results)
    {

        V3d<float> total_force;
        V3d<float> total_moment;
        results.clear();

        for (auto iter = contact_points.begin(); iter != contact_points.end(); ++iter)
        {
            const ContactPoint* cp = *iter;
            ContactResult result = _calculate_single_contact(state, *cp, world, cg_offset);
            results.push_back(result);

            if (result.in_contact)
            {
                total_force += result.force_body;   // Accumulate forces
                total_moment += result.moment_body; // Accumulate moments
            }
        }

        forces_body = total_force;
        moments_body = total_moment;
    }

    // Calculate forces for a single contact point.
    ContactResult _calculate_single_contact(const StateVector<float> &state, const ContactPoint &cp, const World &world, float cg_offset)
    {

        ContactResult result;

        // Get contact parameters for this type
        // params = self._get_contact_params(cp)  just use CP members directly

        // 1. Transform contact point from body frame to earth frame
        auto cp_body = cp.position_body();
        auto orientation = state.orientation();

        // Rotate contact point position to earth frame
        auto cp_earth_offset = orientation.rotate_vector(cp_body);

        // Add aircraft CG position (state position is at datum, need to adjust)
        auto pos = state.position();
        auto cp_earth = pos + cp_earth_offset;
        result.position_earth = cp_earth;

        // 2. Get ground height at contact point
        auto ground_z = world.get_ground_height(cp_earth[0], cp_earth[1]);

        // 3. Calculate penetration (positive = into ground)
        // In NED: Z is down, so penetration = cp_z - ground_z
        auto penetration = cp_earth[2] - ground_z;

        // No contact if above ground
        if (penetration < 0)
            return result;

        result.in_contact = true;
        auto max_pen = cp.max_penetration;
        result.penetration = penetration; // Store actual penetration

        // 4. Calculate contact point velocity in earth frame
        auto cp_vel_body = _get_contact_velocity(state, cp, cg_offset);
        auto cp_vel_earth = orientation.rotate_vector(cp_vel_body);

        // 5. Calculate normal force (spring-damper in earth Z direction)
        // F_normal = k * penetration - c * v_z
        auto k = cp.stiffness;
        auto c = cp.damping;

        // Vertical velocity component (positive = moving down)
        auto v_z = cp_vel_earth[2];

        // Normal force calculation
        // v_z > 0 means moving down, so damping should ADD force (oppose motion)

        auto F_normal = k * penetration + c * v_z; // Normal spring-damper within suspension travel

        // Beyond max penetration: add progressive stiffening
        // This simulates suspension bottoming out on a hard surface
        // Use quadratic stiffening for smoother response
        if (penetration > max_pen)
        {
            float hard_stop_stiffness = 200000.0; // Stiff but not extreme (N/m)
            float hard_stop_damping = 10000.0;    // Additional damping for hard stop
            float excess_penetration = penetration - max_pen;
            // Quadratic stiffening: force increases rapidly with penetration
            F_normal = (k * max_pen +
                        hard_stop_stiffness * excess_penetration * (1 + 10 * excess_penetration) +
                        c * v_z +
                        hard_stop_damping * v_z);
        }

        F_normal = std::max(0.0f, F_normal); // Can only push, not pull
        F_normal = std::min(F_normal, MAX_CONTACT_FORCE);
        result.normal_force = F_normal;

        // 6. Calculate friction forces
        float F_friction_x = 0;
        float F_friction_y = 0;
        _calculate_friction(cp_vel_earth, F_normal, cp, F_friction_x, F_friction_y);
        result.friction_force_long = F_friction_x;
        result.friction_force_lat = F_friction_y;

        // 7. Total force in earth frame
        // Normal force acts upward (-Z), friction acts in XY plane
        auto F_earth = V3d<float>(F_friction_x, F_friction_y, -F_normal);

        // 8. Transform force to body frame
        auto F_body = orientation.rotate_vector_inverse(F_earth);
        result.force_body = F_body;

        // 9. Calculate moment about CG
        // Moment arm from CG to contact point in body frame
        // CG is at (cg_offset, 0, 0) relative to datum
        auto arm = V3d<float>(
            cp.x - cg_offset,
            cp.y,
            cp.z);

        // Moment = arm x force (cross product)
        auto M_x = arm[1] * F_body[2] - arm[2] * F_body[1]; // Roll moment
        auto M_y = arm[2] * F_body[0] - arm[0] * F_body[2]; // Pitch moment
        auto M_z = arm[0] * F_body[1] - arm[1] * F_body[0]; // Yaw moment

        result.moment_body = V3d<float>(M_x, M_y, M_z);

        return result;
    }

    // Calculate velocity of contact point in body frame.
    // V_cp = V_cg + omega x r_cp
    // where r_cp is vector from CG to contact point
    V3d<float> _get_contact_velocity(const StateVector<float> &state, const ContactPoint &cp, float cg_offset)
    {

        auto u = state.velocity()[0];
        auto v = state.velocity()[1];
        auto w = state.velocity()[2];
        auto p = state.angular_velocity()[0];
        auto q = state.angular_velocity()[1];
        auto r = state.angular_velocity()[2];

        // Vector from CG to contact point (in body frame)
        // CG is at (cg_offset, 0, 0) relative to datum
        auto rx = cp.x - cg_offset;
        auto ry = cp.y;
        auto rz = cp.z;

        // omega x r (cross product)
        auto omega_cross_r = V3d<float>(
            q * rz - r * ry,
            r * rx - p * rz,
            p * ry - q * rx);

        // Total velocity at contact point
        return V3d<float>(
            u + omega_cross_r[0],
            v + omega_cross_r[1],
            w + omega_cross_r[2]);
    }

    // Calculate friction forces using Coulomb model with velocity blending.
    // Args:
    //     vel_earth: Contact point velocity in earth frame
    //     normal_force: Normal force magnitude (N)
    //     params: Contact parameters dict
    //     contact_type: 'wheel', 'skid', or 'wingtip'
    // Returns:
    //     (F_x, F_y) friction forces in earth frame
    void _calculate_friction(V3d<float> vel_earth, float normal_force, const ContactPoint &cp, float &F_x, float &F_y)
    {

        // Horizontal velocity components
        auto v_x = vel_earth[0];
        auto v_y = vel_earth[1];
        auto v_horiz = std::sqrt(v_x * v_x + v_y * v_y);

        if (v_horiz < 1e-6 || normal_force < 1e-6)
        {
            F_x = 0.0;
            F_y = 0.0;
            return;
        }

        // Determine friction coefficient
        auto mu_s = cp.friction_static;  // params['friction_static']
        auto mu_d = cp.friction_dynamic; // params['friction_dynamic']

        // Smooth transition between static and dynamic friction
        auto mu = mu_d;
        if (v_horiz < FRICTION_VELOCITY_THRESHOLD)
        {
            // Blend from static to dynamic
            auto blend = v_horiz / FRICTION_VELOCITY_THRESHOLD;
            mu = mu_s * (1 - blend) + mu_d * blend;
        }

        // Maximum friction force
        auto F_friction_max = mu * normal_force;

        // Special handling for wheels (rolling vs sliding)
        if (cp.contact_type == ContactPoint::ContactType::WHEEL)
        {
            // Longitudinal: rolling resistance (small, opposes motion)
            auto F_roll = ROLLING_RESISTANCE * normal_force;
            // Smooth application of rolling resistance
            if (abs(v_x) < FRICTION_VELOCITY_THRESHOLD)
            {
                F_x = -v_x * (F_roll / FRICTION_VELOCITY_THRESHOLD);
            }
            else
            {
                F_x = -copysign(F_roll, v_x);
            }

            // Lateral: full friction (wheels don't roll sideways)
            if (abs(v_y) < FRICTION_VELOCITY_THRESHOLD)
            {
                // Proportional friction at low speed (prevents jitter)
                F_y = -v_y * (F_friction_max / FRICTION_VELOCITY_THRESHOLD);
            }
            else
            {
                F_y = -copysign(F_friction_max, v_y);
            }
        }
        else
        {
            // Skids and wingtips: friction in both directions
            // Direction of friction opposes velocity
            F_x = -F_friction_max * (v_x / v_horiz);
            F_y = -F_friction_max * (v_y / v_horiz);
        }

        // Clamp to max force
        F_x = std::max(-MAX_CONTACT_FORCE, std::min(MAX_CONTACT_FORCE, F_x));
        F_y = std::max(-MAX_CONTACT_FORCE, std::min(MAX_CONTACT_FORCE, F_y));

        return;
    }
};