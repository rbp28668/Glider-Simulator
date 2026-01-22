    
#pragma once

const static double PI = 3.14159265358979323846;

float radians(float degrees){
    return degrees * float(PI) / 180.0f;
}

float degrees(float radians){
    return radians * 180.0f / float(PI);
}

double radians(double degrees) {
    return degrees * PI / 180.0;
}

double degrees(double radians) {
    return radians * 180.0 / PI;
}