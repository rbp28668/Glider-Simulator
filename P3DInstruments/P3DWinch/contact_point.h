#pragma once
#include "sim_types.h"
#include "v3d.h"

//    Class representing a contact point in 3D space for use in ground detection.
//     Position is in body frame relative to datum (root leading edge on fuselage centerline):
//     - x: positive forward
//     - y: positive right (starboard)
//     - z: positive down
class ContactPoint
{

    public:
    enum ContactType
    {
        WHEEL,  // Landing gear wheel (rolling resistance longitudinally, full friction laterally)
        SKID,   // Tail skid or similar (full friction both directions)
        WINGTIP // Wing tip contact (softer, full friction)
    };

    // Position relative to datum in body frame
    float x;
    float y;
    float z;

    ContactType contact_type;

    float stiffness;
    float damping;
    float friction_static;
    float friction_dynamic;
    float max_penetration;
    bool has_brake = false;

    public:

    ContactPoint(float x, float y, float z,
                 ContactType contact_type,
                 float stiffness,
                 float damping,
                 float friction_static,
                 float friction_dynamic,
                 float max_penetration)
        : x(x), y(y), z(z),
          contact_type(contact_type),
          stiffness(stiffness),
          damping(damping),
          friction_static(friction_static),
          friction_dynamic(friction_dynamic),
          max_penetration(max_penetration)
    {
    }

    V3d<NumberT> position_body() const
    {
        // Return position as tuple for vector operations.
        return V3d<NumberT>(x, y, z);
    }
};
