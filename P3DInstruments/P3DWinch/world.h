#pragma once

#include "v3d.h"
#include "local_math.h"

//  Represents the environmental conditions of the simulation world.
class World {
 
    const float gravity = 9.81f;  // m/s² 
    const float air_density = 1.225f;  // kg/m³ at sea level
     
    // Wind vector
    float wx = 0.0f;
    float wy = 0.0f;
    float wz = 0.0f;

    float ground_height = 0.0f;

public:

    World() {
        // Default constructor
    }

    World& operator = (const World& other) {
        wx = other.wx;
        wy = other.wy;
        wz = other.wz;
        ground_height = other.ground_height;
        return *this;
    }

    inline float G() const { return gravity; }

    inline float AirDensity() const { return air_density; }

    // Set the wind conditions in the world.
    // Args:
    //     speed: Wind speed in m/s
    //     direction: Wind direction in degrees from north
    void set_wind(float speed, float direction, float vertical = 0.0f) {
        auto dir_rad = radians(direction);
        wx = speed * cos(dir_rad);
        wy = speed * sin(dir_rad);
        wz = vertical;
    }

    // Get the wind vector in Cartesian coordinates.
    // Args:
    //     position: A tuple (x, y, z) representing the position in meters.
    //     t: Current time in seconds.
    // Returns:
    //     A tuple (wx, wy, wz) representing the wind vector components in m/s.
    V3d<float> get_wind_vector(const V3d<float>& position, float t) const {
       return V3d<float>(wx, wy, wz); 
    }

    void set_wind_vector(float wx, float wy, float wz) {
        this->wx = wx;
        this->wy = wy;
        this->wz = wz;
    }

    // Get the ground height at the current position
    float get_ground_height() const {
        return ground_height;
    }

    void set_ground_height(float height) {
        this->ground_height = height;
    }
};