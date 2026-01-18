#pragma once
#include "aerofoil.h"

class Aerofoil_FX_60_126 :
    public Aerofoil
{
    static float data[][4];
    // 193 data points
    const int line_count = 193;

public:
    Aerofoil_FX_60_126();


    
};

