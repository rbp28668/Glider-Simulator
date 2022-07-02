#pragma once
#include "../P3DCommon/Prepar3D.h"

class SimState;
class Failures;
class SimObjectDataRequest;
class Logger;
class P3DEventCommand;
class IGCFlightRecorder;

class Simulator : public Prepar3D
{
	SimState* state;
	SimObjectDataRequest* stateRequest;
	Failures* failures;
	SimObjectDataRequest* failuresRequest;
	Logger* logger;
	P3DEventCommand* commands;
	IGCFlightRecorder* fr;


public:
	Simulator(const char* appName, bool verbose = false);
	~Simulator();

	SimState* getState() { return state; }
	Failures* getFailures() { return failures; }
	Logger* getLogger() { return logger; }
	P3DEventCommand* getCommands() { return commands; }
	IGCFlightRecorder* getFlightRecorder() { return fr; }
};

