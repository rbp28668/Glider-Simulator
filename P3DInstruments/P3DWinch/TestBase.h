#pragma once

#include<iostream>
#include<assert.h>

#include "v3d.h"
#include  "state_vector.h"
#include  "model.h"
#include  "ask21.h"
#include  "world.h"
#include  "control_inputs.h"
class TestBase
{

protected:
	StateVector<float> state;
	Model model;
	ASK21 aircraft;
	World world;
	ControlInputs controls;


	void assertLess(float first, float second, const char* msg) {
		if (!(first < second)) {
			std::cout << "FAIL: " << msg << std::endl;
		}
	}

	void assertGreater(float first, float second, const char* msg) {
		if (!(first > second)) {
			std::cout << "FAIL: " << msg << std::endl;
		}
	}

	void assertEqual(float first, float second, const char* msg) {
		if (!(first == second)) {
			std::cout << "FAIL: " << msg << std::endl;
		}
	}

	void assertTrue(bool truthy, const char* msg) {
		if (!truthy) {
			std::cout << "FAIL: " << msg << std::endl;
		}
	}

};

