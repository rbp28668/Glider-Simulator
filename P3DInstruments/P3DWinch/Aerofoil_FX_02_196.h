#pragma once
#include "aerofoil.h"

class Aerofoil_FX_02_196 :
    public Aerofoil
{
    static float data[][4];
    // 207 data points
    static const int line_count = 207;

public:
    Aerofoil_FX_02_196();
};

