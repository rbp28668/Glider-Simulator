#pragma once

#include "stdafx.h"
#include <SimConnect.h>
#include <vector>

// So far haven't been able to get a callback with external sim.
// Needs further investigation so in the meantime, keep using
// triggering from a data request.
// #define USE_EXTERNAL_SIM

#include "SimObject.h"
#include "SimObjectList.h"
#include "SimObjectDataRequestList.h"
#include "WeatherStation.h"
#include "WeatherStations.h"

class SimObjectDataRequest;
class ExternalSim;



// Encapsulate the core P3D interface.
class Prepar3D
{
	bool connected;		// true if successfully connected.
	bool opened;		// has received an open event.
	bool started;		// has received a start event, set false on a stop event
	bool paused;		// pause state.
	bool crashed;		// set true if crashed
	bool scenarioRunning; // true if the current scenario is running.
	bool quit;			// set to true to terminate dispatch loop.
	bool verbose;		// set to true for verbose output.
	HANDLE hSimConnect; // use SimConnect_Open to set this value.

	// Provides a sequence number for calls.
	LONG volatile requestIdSequence;

	SimObjectList simObjects; // keyed by object ID (eventually).
	SimObjectDataRequestList dataRequests; // keyed by request ID
	bool waitingDataRequests;
	WeatherStations wxStations;
	ExternalSim* extSim;
	SimObject userAc;

    static void CALLBACK DispatchCallback(SIMCONNECT_RECV *pData, DWORD cbData, void *pContext);
    void Process(SIMCONNECT_RECV *pData, DWORD cbData);

	// Event IDs used to keep track of the sim state.
	enum EVENT_ID{ 
		EVENT_SIM_START, 
		EVENT_SIM_STOP,
		EVENT_PAUSE,
		EVENT_SIM,
		EVENT_CRASHED,
		EVENT_CRASH_RESET,
	}; 

	void connect(const char* pszAppName);
	void registerSystemEvents();
	void subscribeToSystemEvent(EVENT_ID event, const char* name, const char* pszFailureMessage);
	void logEvent(const char* pszEvent, DWORD dwState = 0);
	void showVersionInformation(SIMCONNECT_RECV* pData);
	void handleSystemEvent(SIMCONNECT_RECV* pData);
	void handleException(SIMCONNECT_RECV* pData);
	void handleSimObjectData(SIMCONNECT_RECV* pData);
	void handleSimObjectDataByType(SIMCONNECT_RECV* pData);
	void handleAssignedObjectId(SIMCONNECT_RECV* pData);
	void handleWeatherObservation(SIMCONNECT_RECV* pData);
	
#ifdef USE_EXTERNAL_SIM
	void handleExternalSimCreate(SIMCONNECT_RECV_EXTERNAL_SIM_CREATE* pData);
	void handleExternalSimDestroy(SIMCONNECT_RECV_EXTERNAL_SIM_DESTROY* pData);
	void handleExternalSimSimulate(SIMCONNECT_RECV_EXTERNAL_SIM_SIMULATE* pData);
	void handleExternalSimLocationChanged(SIMCONNECT_RECV_EXTERNAL_SIM_LOCATION_CHANGED* pData);
	void handleExternalSimEvent(SIMCONNECT_RECV_EXTERNAL_SIM_EVENT* pData);
#endif


public:

	const static char* DOCUMENTS;

	void showLastRequest(const char* name);
	HANDLE getHandle() const { return hSimConnect; } 
	bool isPaused() const { return paused; }
	bool isStarted() const { return started; }
	bool isScenarioRunning() const { return scenarioRunning; }
	bool isCrashed() const { return crashed; }
	bool isVerbose() const { return verbose; } 
	void setVerbose(bool isVerbose) { verbose = isVerbose; }

	WeatherStations& weatherStations() { return wxStations; }
	ExternalSim& externalSim() { return *extSim; }
	SimObject& userAircraft() { return userAc; }

    void Dispatch();
	void DispatchLoop();

	// Threadsafe counter for allocating request IDs.
	LONG nextRequestId();

	size_t registerDataRequest(SimObjectDataRequest* pRequest);
	void unregisterDataRequest(SimObjectDataRequest* pRequest);

	void registerSimObject(SimObject* pObject, DWORD dwRequestId);
	SimObject* lookupSimObject(DWORD dwObjectId);
	void unregisterSimObject(DWORD dwObjectId);

	Prepar3D(const char* appName, bool verbose = false);
	virtual ~Prepar3D(void);
};

