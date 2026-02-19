#pragma once

#include <algorithm>

class ControlInputs{

public:
    float aileron;   // [-1, +1] left/right
    float elevator;  // [-1, +1] fwd/aft
    float rudder;    // [-1, +1] left/right
    float spoiler;  // [0, 1] retracted/extended

    ControlInputs(){
        aileron = 0.0;
        elevator = 0.0;
        rudder = 0.0;
        spoiler = 0.0;
    }
        
    void set_controls(float elevator, float aileron, float rudder, float spoiler){
        this->elevator = std::max(-1.0f, std::min(1.0f, elevator));
        this->aileron = std::max(-1.0f, std::min(1.0f, aileron));
        this->rudder = std::max(-1.0f, std::min(1.0f, rudder));
        this->spoiler = std::max(0.0f, std::min(1.0f, spoiler));
    }

};
 