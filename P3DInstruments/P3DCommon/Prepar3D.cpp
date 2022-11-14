#include "stdafx.h"
#include <SimConnect.h>
#include <assert.h>
#include <iostream>
#include <sstream>
#include <list>
#include "Prepar3D.h"
#include "SimObjectDataRequest.h"
#include "Metar.h"
#include "WeatherStations.h"
#include "ExternalSim.h"


// Folder in documents that P3D uses for its files.
const char* Prepar3D::DOCUMENTS = "Prepar3D v4 Files";

/// <summary>
/// Event definitions.  Used as a lookup table to convert event IDs to the corresponding strings
/// and implement reference counting for each event as P3D only allows a client to register
/// and event once.
/// </summary>
Prepar3D::EventDefinition Prepar3D::definitions[to_underlying(EVENT_ID::LAST_P3D_EVENT)] = {
	{ EVENT_ID::EVENT_1SEC, "1sec", 0 },
	{ EVENT_ID::EVENT_4SEC, "4sec", 0 },
	{ EVENT_ID::EVENT_6HZ, "6Hz", 0 },
	{ EVENT_ID::EVENT_AIRCRAFT_LOADED, "AircraftLoaded", 0 },
	{ EVENT_ID::EVENT_CRASHED, "Crashed", 0 },
	{ EVENT_ID::EVENT_CRASH_RESET, "CrashReset", 0 },
	{ EVENT_ID::EVENT_FLIGHT_LOADED, "FlightLoaded", 0 },
	{ EVENT_ID::EVENT_FLIGHT_SAVED, "FlightSaved", 0 },
	{ EVENT_ID::EVENT_FLIGHT_PLAN_ACTIVATED, "FlightPlanActivated", 0 },
	{ EVENT_ID::EVENT_FLIGHT_PLAN_DEACTIVATED, "FlightPlanDeactivated", 0 },
	{ EVENT_ID::EVENT_FRAME, "Frame", 0 },
	{ EVENT_ID::EVENT_PAUSE, "Pause", 0 },
	{ EVENT_ID::EVENT_PAUSED, "Paused", 0 },
	{ EVENT_ID::EVENT_PAUSE_FRAME, "PauseFrame", 0 },
	{ EVENT_ID::EVENT_POSITION_CHANGED, "PositionChanged", 0 },
	{ EVENT_ID::EVENT_SIM, "Sim", 0 },
	{ EVENT_ID::EVENT_SIM_START, "SimStart", 0 },
	{ EVENT_ID::EVENT_SIM_STOP, "SimStop", 0 },
	{ EVENT_ID::EVENT_SOUND, "Sound", 0 },
	{ EVENT_ID::EVENT_UNPAUSED, "Unpaused", 0 },
	{ EVENT_ID::EVENT_VIEW, "View", 0 },
	{ EVENT_ID::EVENT_WEATHER_MODE_CHANGED, "WeatherModeChanged", 0 },
	{ EVENT_ID::EVENT_TEXT_EVENT_CREATED, "TextEventCreated", 0 },
	{ EVENT_ID::EVENT_TEXT_EVENT_DESTROYED, "TextEventDestroyed", 0 },
	{ EVENT_ID::EVENT_OBJECT_ADDED, "ObjectAdded", 0 },
	{ EVENT_ID::EVENT_OBJECT_REMOVED, "ObjectRemoved", 0},

};


Prepar3D::Prepar3D(const char* appName, bool verbose)
	: hSimConnect(NULL),
	connected(false),
	started(false),
	crashed(false),
	paused(false),
	opened(false),
	scenarioRunning(false),
	quit(false),
	verbose(verbose),
	waitingDataRequests(false),
	requestIdSequence(0),
	simObjects(this),
	wxStations(this),
	extSim(0),
	userAc(this)
{

	// Create lookup tables for event definitions
	for (int i = 0; i < to_underlying(EVENT_ID::LAST_P3D_EVENT); ++i) {
		EventDefinition* def = definitions + i;
		idToEvent.insert(std::make_pair(def->id, def));
		nameToEvent.insert(std::make_pair(def->name, def));
	}

	connect(appName);
}


Prepar3D::~Prepar3D(void)
{
	if (extSim) {
		delete extSim;
	}

	HRESULT hr = ::SimConnect_Close(hSimConnect);
}

// Tries to connect to P3d for a while then exits if it fails.
void Prepar3D::connect(const char* appName)
{
	int attemptCount = 0;
	while (!connected) {
		std::cout << "Connecting" << std::endl;
		// Note SIMCONNECT_OPEN_CONFIGINDEX_LOCAL to force connection onto local machine irrespective of simconnect file.
		if (connected = SUCCEEDED(::SimConnect_Open(&hSimConnect, appName, NULL, 0, 0, SIMCONNECT_OPEN_CONFIGINDEX_LOCAL))) {
			showLastRequest("Connected to Prepar3D");
			registerSystemEvents();
			weatherStations().refresh(); // make sure global is initialised initially
			extSim = new ExternalSim(this);
			userAc.setObjectId(SIMCONNECT_OBJECT_ID_USER);
			userAc.setName("User Aircraft");
		}
		else {  // Failed to connect on this attempt
			if (attemptCount >= 10) {
				break;
			}
			else {
				std::cerr << "Not connected, retrying in 2s" << std::endl;
				Sleep(2000);
			}
		}
		++attemptCount;
	}

	if (!connected) {
		std::cerr << "Cannot connect to Prepar3D" << std::endl;
		exit(1);
	}

}

void Prepar3D::registerSystemEvents()
{
	// Subscribe to system events.
	// See https://www.prepar3d.com/SDKv4/sdk/simconnect_api/references/general_functions.html#SimConnect_SubscribeToSystemEvent

	subscribeToSystemEvent(EVENT_ID::EVENT_SIM_START, "SimStart", "Unable to subscribe to SimStart");
	subscribeToSystemEvent(EVENT_ID::EVENT_SIM_STOP, "SimStop", "Unable to subscribe to SimStop");
	subscribeToSystemEvent(EVENT_ID::EVENT_PAUSE, "Pause", "Unable to subscribe to Pause");
	subscribeToSystemEvent(EVENT_ID::EVENT_SIM, "Sim", "Unable to subscribe to Sim");
	subscribeToSystemEvent(EVENT_ID::EVENT_CRASHED, "Crashed", "Unable to subscribe to Crashed");
	subscribeToSystemEvent(EVENT_ID::EVENT_CRASH_RESET, "CrashReset", "Unable to subscribe to CrashReset");
	subscribeToSystemEvent(EVENT_ID::EVENT_OBJECT_ADDED, "ObjectAdded", "Unable to subscribe to ObjectAdded");
	subscribeToSystemEvent(EVENT_ID::EVENT_OBJECT_REMOVED, "ObjectRemoved", "Unable to subscribe to ObjectRemoved");
}

// Checks for failure code, outputs the given message if necessary and exits if failed.
void Prepar3D::subscribeToSystemEvent(EVENT_ID event, const char* name, const char* pszFailureMessage)
{
	LONG id = subscribeToSystemEvent(name);
	if(id == to_underlying(event)) {
		if (verbose) {
			std::ostringstream os;
			os << "Registered event " << name << " with id " << to_underlying(event);
			showLastRequest(os.str().c_str());
		}
	}
	else
	{
		std::cerr << pszFailureMessage << std::endl;
		exit(2);
	}
}

// Log an event to stdout.
void Prepar3D::logEvent(const char* pszEvent, DWORD dwState)
{
	if (isVerbose()) {
		std::cout << pszEvent << " " << dwState << std::endl;
	}
}

void Prepar3D::showVersionInformation(SIMCONNECT_RECV* pData)
{
	SIMCONNECT_RECV_OPEN* pOpen = (SIMCONNECT_RECV_OPEN*)pData;
	std::cout << pOpen->szApplicationName << " "
		<< pOpen->dwApplicationVersionMajor << "." << pOpen->dwApplicationVersionMinor
		<< " Build " << pOpen->dwApplicationBuildMajor << "." << pOpen->dwApplicationBuildMinor << std::endl;
	std::cout << "Sim Connect version " << pOpen->dwSimConnectVersionMajor << "." << pOpen->dwSimConnectBuildMinor << std::endl;
}

void Prepar3D::handleSystemEvent(SIMCONNECT_RECV* pData)
{
	SIMCONNECT_RECV_EVENT* evt = (SIMCONNECT_RECV_EVENT*)pData;

	switch (evt->uEventID) {
	case to_underlying(EVENT_ID::EVENT_SIM_START):
		logEvent("Sim Start");
		// Defer creation of SimConnect requests until the sim proper has started.
		dataRequests.createRequests();
		waitingDataRequests = false;
		weatherStations().refresh();
		started = true;

		if (isVerbose()) {
			std::cout << "Data Requests Created" << std::endl;
		}
		break;

	case to_underlying(EVENT_ID::EVENT_SIM_STOP):
		logEvent("Sim Stop");
		started = false;
		break;

	case to_underlying(EVENT_ID::EVENT_PAUSE):
		logEvent("Paused", evt->dwData);
		paused = (evt->dwData != 0);
		if (!paused) {
			weatherStations().refresh();
		}
		break;

	case to_underlying(EVENT_ID::EVENT_SIM):
		logEvent("Sim running", evt->dwData);
		scenarioRunning = (evt->dwData != 0);
		if (scenarioRunning && waitingDataRequests) {
			dataRequests.createRequests();
			waitingDataRequests = false;
		}
		weatherStations().refresh();
		break;

	case to_underlying(EVENT_ID::EVENT_CRASHED):
		logEvent("Crashed");
		crashed = true;
		break;

	case to_underlying(EVENT_ID::EVENT_CRASH_RESET):
		logEvent("Crash Reset");
		crashed = false;
		break;

	default:
		break;

	}

	// Allow any event handlers to receive the event.
	for (auto iter = systemEventHandlers.begin(); iter != systemEventHandlers.end(); ++iter) {
		(*iter)->handleEvent(evt);
	}

}

void Prepar3D::handleException(SIMCONNECT_RECV* pData)
{
	SIMCONNECT_RECV_EXCEPTION* except = (SIMCONNECT_RECV_EXCEPTION*)pData;
	std::cerr << "Exception " << except->dwException << "\n";
	std::cerr << "ID " << except->dwID << "\n";
	std::cerr << "Index " << except->dwIndex << "\n";

	switch (except->dwException) {
	case SIMCONNECT_RECV_EXCEPTION::UNKNOWN_INDEX:
		std::cerr << "Unknown Index" << std::endl;
		break;

	case SIMCONNECT_RECV_EXCEPTION::UNKNOWN_SENDID:
		std::cerr << "Unknown sender ID" << std::endl;
		break;
	}
}

void Prepar3D::handleSimObjectData(SIMCONNECT_RECV* pData)
{
	SIMCONNECT_RECV_SIMOBJECT_DATA* pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA*)pData;
	SimObjectDataRequest* pRequest = dataRequests[pObjData->dwRequestID];
	
	if (pRequest != 0) {
		pRequest->handle(pObjData);
	}
	else {
		std::cerr << "Sim Object Data: Unknown request ID: " << pObjData->dwRequestID << std::endl;
	}
}

void Prepar3D::handleSimObjectDataByType(SIMCONNECT_RECV* pData)
{
	SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE* pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE*)pData;
	SimObjectDataRequest* pRequest = dataRequests[pObjData->dwRequestID];

	if (pRequest != 0) {
		pRequest->handle(pObjData);
	}
	else {
		std::cerr << "Sim Object Data By Type: Unknown request ID: " << pObjData->dwRequestID << std::endl;
	}
}

void Prepar3D::handleAssignedObjectId(SIMCONNECT_RECV* pData)
{
	assert(this);
	assert(pData);
	assert(pData->dwID == SIMCONNECT_RECV_ID_ASSIGNED_OBJECT_ID);

	SIMCONNECT_RECV_ASSIGNED_OBJECT_ID* paoid = static_cast<SIMCONNECT_RECV_ASSIGNED_OBJECT_ID*>(pData);
	DWORD objectId = paoid->dwObjectID;
	DWORD requestId = paoid->dwRequestID;
	simObjects.associate(requestId, objectId);
	if (verbose) {
		std::cout << "Associated object ID " << objectId << " to request " << requestId << std::endl;
	}
}

void Prepar3D::handleWeatherObservation(SIMCONNECT_RECV* pData)
{
	SIMCONNECT_RECV_WEATHER_OBSERVATION* pwxData = (SIMCONNECT_RECV_WEATHER_OBSERVATION*)pData;
	const char* pszMETAR = (const char*) & (pwxData->szMetar);
	wxStations.update(pszMETAR);
	if (verbose) {
		std::cout << pszMETAR << std::endl;
	}
}

void Prepar3D::handleObjectAddRemove(SIMCONNECT_RECV* pData) {
	assert(this);
	assert(pData);
	assert(pData->dwID == SIMCONNECT_RECV_ID_EVENT_OBJECT_ADDREMOVE);

	SIMCONNECT_RECV_EVENT_OBJECT_ADDREMOVE* poar = static_cast<SIMCONNECT_RECV_EVENT_OBJECT_ADDREMOVE*>(pData);

	if (poar->eObjType == SIMCONNECT_SIMOBJECT_TYPE_AIRCRAFT) {
		if (poar->uEventID == to_underlying(EVENT_ID::EVENT_OBJECT_ADDED)) {
			aircraftAdded(poar->dwData);
		}
		else if (poar->uEventID == to_underlying(EVENT_ID::EVENT_OBJECT_REMOVED)) {
			aircraftRemoved(poar->dwData);
		}
	}
}

#ifdef USE_EXTERNAL_SIM
void Prepar3D::handleExternalSimCreate(SIMCONNECT_RECV_EXTERNAL_SIM_CREATE* pData)
{
	extSim->create(pData);
	::SimConnect_SynchronousUnblock(hSimConnect);
	if (verbose) {
		std::cout << "External sim create " << pData->dwObjectID << std::endl;
	}
}

void Prepar3D::handleExternalSimDestroy(SIMCONNECT_RECV_EXTERNAL_SIM_DESTROY* pData)
{
	extSim->destroy(pData);
	::SimConnect_SynchronousUnblock(hSimConnect);
}

void Prepar3D::handleExternalSimSimulate(SIMCONNECT_RECV_EXTERNAL_SIM_SIMULATE* pData)
{
	extSim->simulate(pData);
	::SimConnect_SynchronousUnblock(hSimConnect);
	if (verbose) {
		std::cout << "External sim simulate " << pData->dwObjectID << std::endl;
	}
}

void Prepar3D::handleExternalSimLocationChanged(SIMCONNECT_RECV_EXTERNAL_SIM_LOCATION_CHANGED* pData)
{
	extSim->locationChanged(pData);
	::SimConnect_SynchronousUnblock(hSimConnect);
}

void Prepar3D::handleExternalSimEvent(SIMCONNECT_RECV_EXTERNAL_SIM_EVENT* pData)
{
	extSim->event(pData);
	::SimConnect_SynchronousUnblock(hSimConnect);
}
#endif

void Prepar3D::showLastRequest(const char* name)
{
	if (verbose) {
		DWORD sendId;
		HRESULT hr = ::SimConnect_GetLastSentPacketID(hSimConnect, &sendId);
		std::cout << name << " has request ID " << sendId << std::endl;
	}
}

void Prepar3D::Dispatch()
{
	::SimConnect_CallDispatch(hSimConnect, &Prepar3D::DispatchCallback, this);
}

void Prepar3D::DispatchLoop() {

	while (0 == quit) {
		Dispatch();
		Sleep(1);
	}
}

// static callback function uses the context variable to identify this class instance.
void Prepar3D::DispatchCallback(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext)
{
	Prepar3D* pThis = reinterpret_cast<Prepar3D*>(pContext);
	pThis->Process(pData, cbData);
}

// OO version of the callback.
void Prepar3D::Process(SIMCONNECT_RECV* pData, DWORD cbData)
{
	//std::cout << "Request type " << pData->dwID << std::endl;
	
	switch (pData->dwID)
	{
	case SIMCONNECT_RECV_ID_EXCEPTION:
		handleException(pData);
		break;


		// See https://www.prepar3d.com/SDKv4/sdk/simconnect_api/references/structures_and_enumerations.html#SIMCONNECT_RECV_OPEN
	case SIMCONNECT_RECV_ID_OPEN:
		logEvent("Open");
		showVersionInformation(pData);
		break;

	case SIMCONNECT_RECV_ID_EVENT:
		handleSystemEvent(pData);
		break;

	case SIMCONNECT_RECV_ID_SIMOBJECT_DATA:
		handleSimObjectData(pData);
		break;

	case SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE:
		handleSimObjectDataByType(pData);
		break;

	case SIMCONNECT_RECV_ID_ASSIGNED_OBJECT_ID:
		handleAssignedObjectId(pData);
		break;

	case SIMCONNECT_RECV_ID_WEATHER_OBSERVATION:
		handleWeatherObservation(pData);
		break;

	case SIMCONNECT_RECV_ID_EVENT_OBJECT_ADDREMOVE:
		handleObjectAddRemove(pData);
		break;

#ifdef USE_EXTERNAL_SIM
	case SIMCONNECT_RECV_ID_EXTERNAL_SIM_CREATE:
		handleExternalSimCreate((SIMCONNECT_RECV_EXTERNAL_SIM_CREATE*)pData);
		break;

	case SIMCONNECT_RECV_ID_EXTERNAL_SIM_DESTROY:
		handleExternalSimDestroy((SIMCONNECT_RECV_EXTERNAL_SIM_DESTROY*)pData);
		break;

	case SIMCONNECT_RECV_ID_EXTERNAL_SIM_SIMULATE:
		handleExternalSimSimulate((SIMCONNECT_RECV_EXTERNAL_SIM_SIMULATE*)pData);
		break;

	case SIMCONNECT_RECV_ID_EXTERNAL_SIM_LOCATION_CHANGED:
		handleExternalSimLocationChanged((SIMCONNECT_RECV_EXTERNAL_SIM_LOCATION_CHANGED*)pData);
		break;

	case SIMCONNECT_RECV_ID_EXTERNAL_SIM_EVENT:
		handleExternalSimEvent((SIMCONNECT_RECV_EXTERNAL_SIM_EVENT*)pData);
		break;
#endif

	case SIMCONNECT_RECV_ID_QUIT:
		std::cout << "Sim quit" << std::endl;
		quit = true;

		// Let any registered handlers we're closing down.
		for (auto iter = systemEventHandlers.begin(); iter != systemEventHandlers.end(); ++iter) {
			(*iter)->quitEvent();
		}

		break;

	default:
		std::cout << "Received unknown event ID: " << pData->dwID << std::endl;
		break;
	}
}

// Gets a sequential ID for requests to P3D.
LONG Prepar3D::nextRequestId()
{
	return ::InterlockedIncrement(&requestIdSequence);
}


// Registers a data request so that the actual request to P3D can be
// created when need be.
size_t Prepar3D::registerDataRequest(SimObjectDataRequest* pRequest) {
	assert(pRequest != 0);

	dataRequests.add(pRequest, pRequest->getId());
	waitingDataRequests = true;
	// If already started register (or re-register) any requests.
	if (started || scenarioRunning) {
		dataRequests.createRequests();
		waitingDataRequests = false;
	}

	return pRequest->getId();
}

// Unregisters a data request so that it's no longer active.
void Prepar3D::unregisterDataRequest(SimObjectDataRequest* pRequest) {
	assert(pRequest != 0);
	DWORD id = pRequest->getId();
	dataRequests.remove(id);
}

void Prepar3D::registerSimObject(SimObject* pObject, DWORD dwRequestId)
{
	assert(this);
	assert(pObject);
	pObject->setRequestId(dwRequestId);
	simObjects.add(pObject, dwRequestId);
}

SimObject* Prepar3D::lookupSimObject(DWORD dwObjectId)
{
	return simObjects.lookup(dwObjectId);
}

void Prepar3D::unregisterSimObject(DWORD dwObjectId)
{
	if(dwObjectId != userAc.id()){
		simObjects.remove(dwObjectId);
	}
}

/// <summary>
/// Registers an event handler to receive any system events.
/// Threadsafe.  After registering the handler will receive
/// all system events.
/// </summary>
/// <param name="handler">is the event handler to register.</param>
void Prepar3D::registerSystemEventHandler(SystemEventHandler* handler)
{
	CriticalSection::Lock lock(eventHandersGuard);
	systemEventHandlers.push_back(handler);
}

/// <summary>
/// Un-registers a system event handler. Threadsafe.
/// </summary>
/// <param name="handler">is the handler to unregister</param>
void Prepar3D::unRegisterSystemEventHandler(SystemEventHandler* handler)
{
	CriticalSection::Lock lock(eventHandersGuard);
	systemEventHandlers.remove(handler);
}

/// <summary>
/// Sets up a subscription to a given system event.  Subscribed events will be sent
/// to any registered event handler.  Note that this reference counts so that if 
/// there are 2 calls to subscribe to a given event it will only be registered on the
/// first call but unsubscribed on the second call to unsubscribeFromSystemEvent.
/// </summary>
/// <param name="name">Is the event name to subscribe to</param>
/// <returns>The event ID or -1 for failure.</returns>
LONG Prepar3D::subscribeToSystemEvent(const char* name)
{
	auto it = nameToEvent.find(name);
	if (it != nameToEvent.end()) {
		CriticalSection::Lock lock(definitionsGuard);
		EventDefinition* def = it->second;
		SIMCONNECT_CLIENT_EVENT_ID id = to_underlying(def->id);

		if (def->registered == 0) {
			HRESULT hr = ::SimConnect_SubscribeToSystemEvent(hSimConnect, id, def->name);
			if (SUCCEEDED(hr)) {
				++def->registered;
				return to_underlying(def->id);
			}
			else { // failed
				return -1;
			}
		}
		else { // registered count > 0 so already registered - just return the ID
			return id;  
		}
	}
	else {
		return -1;
	}
}

/// <summary>
/// Unsubscribes from a system event.  Reference counting ensures that the unsubscribe call
/// to P3D only takes place when there are as many unsubscribe calls for an event as there
/// were subscribe calls.
/// </summary>
/// <param name="eventId">is the event ID returned by subscribeToSystemEvent</param>
void Prepar3D::unsubscribeFromSystemEvent(LONG eventId)
{
	auto it = idToEvent.find((EVENT_ID)eventId);
	if (it != idToEvent.end()) {
		CriticalSection::Lock lock(definitionsGuard);
		EventDefinition* def = it->second;
		if (def->registered == 0) {
			return; 
		}
		--def->registered;
		if (def->registered == 0) {
			::SimConnect_UnsubscribeFromSystemEvent(hSimConnect, eventId);
		}
	}
}

void Prepar3D::aircraftAdded(DWORD objectId)
{
}

void Prepar3D::aircraftRemoved(DWORD objectId)
{
}




