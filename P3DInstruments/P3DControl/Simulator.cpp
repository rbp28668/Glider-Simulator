#include "stdafx.h"
#include <iostream>
#include "Simulator.h"
#include "SimState.h"
#include "Failures.h"
#include "Logger.h"
#include "P3DEventCommand.h"
#include "IGCFlightRecorder.h"
#include "Tug.h"

#include "../P3DCommon/SimObjectDataRequest.h"

Simulator::Simulator(const char* appName, bool verbose)
	:Prepar3D(appName, verbose)
	, state(0)
	, stateRequest(0)
	, failures(0)
	, failuresRequest(0)
	, logger(0)
	, commands(0)
	, fr(0)
	, pTug(0)
{
	logger = new Logger();
	logger->info("Startup");
	state = new SimState(this);
	stateRequest = new SimObjectDataRequest(this, state, &userAircraft(), SIMCONNECT_PERIOD_SECOND);
	failures = new Failures(this);
	failuresRequest = new SimObjectDataRequest(this, failures, &userAircraft(), SIMCONNECT_PERIOD_SECOND);
	commands = new P3DEventCommand(this);
	fr = new IGCFlightRecorder(this);
}

Simulator::~Simulator()
{
	logger->info("Shutdown");
	delete fr;
	delete commands;
	delete failuresRequest;
	delete failures;
	delete stateRequest;
	delete state;
	delete logger;

	SimObject* toDelete = pTug;
	pTug = 0;
	delete toDelete;

}

void Simulator::aircraftAdded(DWORD objectId)
{
	SimObject* existing = lookupSimObject(objectId);
	if (existing == 0) {
		std::cout << "Create simobject aircraft: " << objectId << std::endl; // 31
		std::cout << "maybe towplane" << std::endl;
		if (pTug) {
			delete pTug;
		}

		pTug = new Tug(this, objectId);

		// Try sending flaps up
		//::SimConnect_MapClientEventToSimEvent(getHandle(), 1000, "RUDDER_SET");
		//HRESULT hr = ::SimConnect_TransmitClientEvent(getHandle(), objectId, 1000, 16383, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);

	}

}

void Simulator::aircraftRemoved(DWORD objectId)
{
	if (pTug && objectId == pTug->id()) {
		SimObject* toDelete = pTug;
		pTug = 0;
		delete toDelete;
	}

}

void Simulator::tugReleased()
{
	if (pTug) {
		pTug->released();
	}
}
