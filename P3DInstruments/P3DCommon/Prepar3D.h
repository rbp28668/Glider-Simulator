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


// Get the underlying int (or underlying type) of an enum value.
// Use when you have enum class ... and want to convert to an integer.
template <typename E>
constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept {
	return static_cast<typename std::underlying_type<E>::type>(e);
}


// Encapsulate the core P3D interface.
class Prepar3D
{
public:

	// Callback handler to get data 
	class SystemEventHandler {
	public:
		virtual void handleEvent(SIMCONNECT_RECV_EVENT* evt) = 0;
		virtual void quitEvent() {}
	};

private:


	bool connected;		// true if successfully connected.
	bool opened;		// has received an open event.
	bool started;		// has received a start event, set false on a stop event
	bool paused;		// pause state.
	bool crashed;		// set true if crashed
	bool scenarioRunning; // true if the current scenario is running.
	bool quit;			// set to true to terminate dispatch loop.
	bool verbose;		// set to true for verbose output.
	HANDLE hSimConnect; // use SimConnect_Open to set this value.

	// Provides a sequence number for calls or event IDs.
	LONG volatile requestIdSequence;

	CriticalSection eventHandersGuard;
	std::list<SystemEventHandler*> systemEventHandlers;

	SimObjectList simObjects; // keyed by object ID (eventually).
	SimObjectDataRequestList dataRequests; // keyed by request ID
	bool waitingDataRequests;
	WeatherStations wxStations;
	ExternalSim* extSim;
	SimObject userAc;

    static void CALLBACK DispatchCallback(SIMCONNECT_RECV *pData, DWORD cbData, void *pContext);
    void Process(SIMCONNECT_RECV *pData, DWORD cbData);

public:  // Note event IDs public so can ensure other event IDs start after LAST_P3D_EVENT
	// Event IDs used to keep track of the sim state.
	enum class EVENT_ID : int{
		EVENT_1SEC,
		EVENT_4SEC,
		EVENT_6HZ,
		EVENT_AIRCRAFT_LOADED,
		EVENT_CRASHED,
		EVENT_CRASH_RESET,
		EVENT_FLIGHT_LOADED,
		EVENT_FLIGHT_SAVED,
		EVENT_FLIGHT_PLAN_ACTIVATED,
		EVENT_FLIGHT_PLAN_DEACTIVATED,
		EVENT_FRAME,
		EVENT_PAUSE,
		EVENT_PAUSED,
		EVENT_PAUSE_FRAME,
		EVENT_POSITION_CHANGED,
		EVENT_SIM,
		EVENT_SIM_START,
		EVENT_SIM_STOP,
		EVENT_SOUND,
		EVENT_UNPAUSED,
		EVENT_VIEW,
		EVENT_WEATHER_MODE_CHANGED,
		EVENT_TEXT_EVENT_CREATED,
		EVENT_TEXT_EVENT_DESTROYED,
		EVENT_OBJECT_ADDED,
		EVENT_OBJECT_REMOVED,
		LAST_P3D_EVENT
		}; 
	
private:

	struct EventDefinition {
		enum EVENT_ID id;
		const char* name;
		unsigned int registered; // reference counted.
	};

	CriticalSection definitionsGuard;
	static EventDefinition definitions[to_underlying(EVENT_ID::LAST_P3D_EVENT)];

	// Used to map the IDs and names to the EventDefinition structures.
	// Once created these should never need to be modified.
	std::map<EVENT_ID, EventDefinition*> idToEvent;
	std::map<std::string, EventDefinition*> nameToEvent;
	

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
	void handleObjectAddRemove(SIMCONNECT_RECV* pData);

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


	LONG nextRequestId();	// Threadsafe counter for allocating request IDs.
	//LONG nextEventId();		// Threadsafe counter for allocating event IDs.

	size_t registerDataRequest(SimObjectDataRequest* pRequest);
	void unregisterDataRequest(SimObjectDataRequest* pRequest);

	void registerSimObject(SimObject* pObject, DWORD dwRequestId);
	SimObject* lookupSimObject(DWORD dwObjectId);
	void unregisterSimObject(DWORD dwObjectId);

	// Subscribe/unsubscribe to system events
	LONG subscribeToSystemEvent(const char* name);
	void unsubscribeFromSystemEvent(LONG eventId);

	// Register/unregister event handlers for system events.
	void registerSystemEventHandler(SystemEventHandler* handler);
	void unRegisterSystemEventHandler(SystemEventHandler* handler);

	virtual void aircraftAdded(DWORD objectId);
	virtual void aircraftRemoved(DWORD objectId);

	Prepar3D(const char* appName, bool verbose = false);
	virtual ~Prepar3D(void);
};

