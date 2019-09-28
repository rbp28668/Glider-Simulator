#include "stdafx.h"
#include "Simulator.h"
#include "SimState.h"
#include "../P3DCommon/SimObjectDataRequest.h"

Simulator::Simulator(const char* appName, bool verbose)
	:Prepar3D(appName, verbose)
	, state(0)
	, request(0)
{
	state = new SimState(this);
	request = new SimObjectDataRequest(this, state, SIMCONNECT_PERIOD_SECOND);
	
}

Simulator::~Simulator()
{
	delete request;
	delete state;
}
