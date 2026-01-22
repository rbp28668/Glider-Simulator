#pragma once

#include "v3d.h"
#include "local_math.h"

//  Represents the environmental conditions of the simulation world.
class World {
 
    public:

    const float gravity = 9.81f;  // m/s² 
    const float air_density = 1.225f;  // kg/m³ at sea level
    float wind_speed = 0.0f;  // m/s
    float wind_direction = 0.0f;  // degrees from north

    World() {
        // Default constructor
    }

    // Set the wind conditions in the world.
    // Args:
    //     speed: Wind speed in m/s
    //     direction: Wind direction in degrees from north
    void set_wind(float speed, float direction) {
        wind_speed = speed;
        wind_direction = direction;
    }

    // Get the wind vector in Cartesian coordinates.
    // Args:
    //     position: A tuple (x, y, z) representing the position in meters.
    //     t: Current time in seconds.
    // Returns:
    //     A tuple (wx, wy, wz) representing the wind vector components in m/s.
    V3d<float> get_wind_vector(const V3d<float>& position, float t) const {
        auto dir_rad = radians(wind_direction);
        auto wx = wind_speed * cos(dir_rad);
        auto wy = wind_speed * sin(dir_rad);
        return V3d<float>(wx, wy, 0.0);  // Assuming no vertical wind component
    }

    // Get the ground height at a given (x, y) position.
    // Args:
    //     x: X coordinate in meters
    //     y: Y coordinate in meters
    // Returns:
    //     Ground height (Z) in meters
    float get_ground_height(float x, float y) const {
        return 0.0;  // Flat ground at Z=0 for simplicity
    }
};