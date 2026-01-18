#pragma once
#include "aerofoil.h"

class Aerofoil_NACA0010 :
    public Aerofoil
{
    static float data[][4];
    // 207 data points
    const int line_count = 207;

public:
    Aerofoil_NACA0010();
};

