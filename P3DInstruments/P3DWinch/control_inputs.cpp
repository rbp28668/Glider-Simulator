#pragma once

#include <algorithm>
#include "control_inputs.h"


void ControlInputs::set_controls(float elevator, float aileron, float rudder, float spoiler, float brake, float release) {
	this->elevator = std::max(-1.0f, std::min(1.0f, elevator));
	this->aileron = std::max(-1.0f, std::min(1.0f, aileron));
	this->rudder = std::max(-1.0f, std::min(1.0f, rudder));
	this->spoiler = std::max(0.0f, std::min(1.0f, spoiler));
	this->brake = std::max(0.0f, std::min(1.0f, brake));
	this->release = std::max(0.0f, std::min(1.0f, release));
}


