#pragma once
#include "../P3DCommon/Prepar3D.h"

class SimState;
class Failures;
class SimObjectDataRequest;

class Simulator : public Prepar3D
{
	SimState* state;
	SimObjectDataRequest* stateRequest;
	Failures* failures;
	SimObjectDataRequest* failuresRequest;

public:
	Simulator(const char* appName, bool verbose = false);
	~Simulator();

	SimState* getState() { return state; }
	Failures* getFailures() { return failures; }
};

