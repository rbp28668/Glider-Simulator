#include "stdafx.h"
#include <SimConnect.h>
#include <assert.h>
#include <iostream>
#include <sstream>
#include "Prepar3D.h"
#include "SimObjectDataRequest.h"

// Folder in documents that P3D uses for its files.
const char* Prepar3D::DOCUMENTS = "Prepar3D v4 Files";

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
	simObjects(this)

{	
	connect(appName);
}


Prepar3D::~Prepar3D(void)
{
	HRESULT hr = ::SimConnect_Close(hSimConnect); 
}

// Tries to connect to P3d for a while then exits if it fails.
void Prepar3D::connect(const char* appName)
{
	int attemptCount = 0;
	while (!connected) {
		std::cout << "Connecting" << std::endl;
		if (connected = SUCCEEDED(SimConnect_Open(&hSimConnect, appName, NULL, 0, 0, 0))) {
			showLastRequest("Connected to Prepar3D");
			registerSystemEvents();
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

	subscribeToSystemEvent( EVENT_SIM_START, "SimStart", "Unable to subscribe to SimStart");
	subscribeToSystemEvent( EVENT_SIM_STOP, "SimStop", "Unable to subscribe to SimStop");
	subscribeToSystemEvent( EVENT_PAUSE, "Pause", "Unable to subscribe to Pause");
	subscribeToSystemEvent( EVENT_SIM, "Sim", "Unable to subscribe to Sim");
	subscribeToSystemEvent( EVENT_CRASHED, "Crashed", "Unable to subscribe to Crashed");
	subscribeToSystemEvent( EVENT_CRASH_RESET, "CrashReset", "Unable to subscribe to CrashReset");
}

// Checks for failure code, outputs the given message if necessary and exits if failed.
void Prepar3D::subscribeToSystemEvent(EVENT_ID event, const char* name, const char* pszFailureMessage)
{
	HRESULT hr = ::SimConnect_SubscribeToSystemEvent(hSimConnect, event, name);
	if (SUCCEEDED(hr)) {
		if (verbose) {
			std::ostringstream os;
			os << "Registered event " << name << " with id " << event;
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
	case EVENT_SIM_START:
		logEvent("Sim Start");
		// Defer creation of SimConnect requests until the sim proper has started.
		dataRequests.createRequests();
		waitingDataRequests = false;
		started = true;

		if (isVerbose()) {
			std::cout << "Data Requests Created" << std::endl;
		}
		break;

	case EVENT_SIM_STOP:
		logEvent("Sim Stop");
		started = false;
		break;

	case EVENT_PAUSE:
		logEvent("Paused", evt->dwData);
		paused = (evt->dwData != 0);
		break;

	case EVENT_SIM:
		logEvent("Sim running", evt->dwData);
		scenarioRunning = (evt->dwData != 0);
		if (scenarioRunning && waitingDataRequests) {
			dataRequests.createRequests();
			waitingDataRequests = false;
		}
		break;

	case EVENT_CRASHED:
		logEvent("Crashed");
		crashed = true;
		break;

	case EVENT_CRASH_RESET:
		logEvent("Crash Reset");
		crashed = false;
		break;

	default:
		break;
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
}

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

        while( 0 == quit ) { 
            Dispatch();
            Sleep(1); 
        }  
}

// static callback function uses the context variable to identify this class instance.
void Prepar3D::DispatchCallback(SIMCONNECT_RECV *pData, DWORD cbData, void *pContext)
{
    Prepar3D *pThis = reinterpret_cast<Prepar3D*>(pContext);
    pThis->Process(pData, cbData);
}

// OO version of the callback.
void Prepar3D::Process(SIMCONNECT_RECV *pData, DWORD cbData)
{
    switch(pData->dwID) 
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

    case SIMCONNECT_RECV_ID_QUIT: 
        std::cout << "Sim quit" << std::endl;
        quit = true; 
        break; 
     
    default: 
        std::cout << "Received unknown event ID: " << pData->dwID << std::endl; 
        break; 
    } 
}

LONG Prepar3D::nextRequestId()
{
	return ::InterlockedIncrement(&requestIdSequence);
}

size_t Prepar3D::registerDataRequest(SimObjectDataRequest* pRequest){
	assert(pRequest != 0);

	long id = nextRequestId();
	dataRequests.add(pRequest,id);
	waitingDataRequests = true;
	// If already started register (or re-register) any requests.
	if(started || scenarioRunning) {
		dataRequests.createRequests();
		waitingDataRequests = false;
	}

	return id;		
}

void Prepar3D::unregisterDataRequest(SimObjectDataRequest* pRequest){
	assert(pRequest != 0);
	DWORD id = pRequest->getId();
	dataRequests.remove(id);
}




