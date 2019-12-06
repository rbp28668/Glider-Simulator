#include "stdafx.h"
#include "Simulator.h"
#include "SimState.h"
#include "Failures.h"
#include "Logger.h"

#include "../P3DCommon/SimObjectDataRequest.h"

Simulator::Simulator(const char* appName, bool verbose)
	:Prepar3D(appName, verbose)
	, state(0)
	, stateRequest(0)
	, failures(0)
	, failuresRequest(0)
	, logger(0)
{
	logger = new Logger();
	logger->info("Startup");
	state = new SimState(this);
	stateRequest = new SimObjectDataRequest(this, state, &userAircraft(), SIMCONNECT_PERIOD_SECOND);
	failures = new Failures(this);
	failuresRequest = new SimObjectDataRequest(this, failures, &userAircraft(), SIMCONNECT_PERIOD_SECOND);
}

Simulator::~Simulator()
{
	logger->info("Shutdown");
	delete failuresRequest;
	delete failures;
	delete stateRequest;
	delete state;
	delete logger;
}
