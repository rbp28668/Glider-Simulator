#pragma once

#include <algorithm>

class ControlInputs{

public:
    float aileron;   // [-1, +1] left/right
    float elevator;  // [-1, +1] fwd/aft
    float rudder;    // [-1, +1] left/right
    float spoiler;  // [0, 1] retracted/extended
    float brake;    // [0, 1] released/full

    ControlInputs(){
        aileron = 0.0;
        elevator = 0.0;
        rudder = 0.0;
        spoiler = 0.0;
        brake = 0.0;
    }

    void set_controls(float elevator, float aileron, float rudder, float spoiler, float brake);

};
 