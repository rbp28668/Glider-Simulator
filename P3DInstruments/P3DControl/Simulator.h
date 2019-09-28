#pragma once
#include "../P3DCommon/Prepar3D.h"

class SimState;
class SimObjectDataRequest;

class Simulator : public Prepar3D
{
	SimState* state;
	SimObjectDataRequest* request;

public:
	Simulator(const char* appName, bool verbose = false);
	~Simulator();

	SimState* getState() { return state; }

};

