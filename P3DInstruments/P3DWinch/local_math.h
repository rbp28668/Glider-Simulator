    
#pragma once

const static double PI = 3.14159265358979323846;

inline float radians(float degrees){
    return degrees * float(PI) / 180.0f;
}

inline float degrees(float radians){
    return radians * 180.0f / float(PI);
}

inline double radians(double degrees) {
    return degrees * PI / 180.0;
}

inline double degrees(double radians) {
    return radians * 180.0 / PI;
}