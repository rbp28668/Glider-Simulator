#pragma once

#include <algorithm>

class ControlInputs{

public:
    float aileron;   // [-1, +1] left/right
    float elevator;  // [-1, +1] fwd/aft
    float rudder;    // [-1, +1] left/right
    float spoiler;  // [0, 1] retracted/extended
    float brake;    // [0, 1] released/full
    float release;  // 

    ControlInputs(){
        aileron = 0.0f;
        elevator = 0.0f;
        rudder = 0.0f;
        spoiler = 0.0f;
        brake = 0.0f;
        release = 0.0f;
    }

    void set_controls(float elevator, float aileron, float rudder, float spoiler, float brake, float release);

};
 