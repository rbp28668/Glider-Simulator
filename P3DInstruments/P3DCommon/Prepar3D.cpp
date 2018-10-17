#include "stdafx.h"
#include <SimConnect.h>
#include <assert.h>
#include <iostream>
#include "Prepar3D.h"
#include "SimObjectDataRequest.h"


Prepar3D::Prepar3D(const char* appName)
	: hSimConnect(NULL),
	connected(false),
	started(false),
	quit(false)
{	
	int attemptCount = 0;
	while(!connected) {
		std::cout << "Connecting" << std::endl;
		if(connected = SUCCEEDED(SimConnect_Open(&hSimConnect, appName, NULL, 0, 0, 0))) {
			std::cout << "Connected to Prepar3D" << std::endl;
			// Request an event when the simulation starts 
			HRESULT hr = SimConnect_SubscribeToSystemEvent(hSimConnect, EVENT_SIM_START, "SimStart"); 
			if(FAILED(hr)) {
				std::cerr << "Unable to subscribe to SimStart" << std::endl;
				exit(2);
			}
		} else  {  // Failed to connect on this attempt
			if(attemptCount >= 10) {
				break;
			} else { 
				std::cerr << "Not connected, retrying in 2s" << std::endl;
				Sleep(2000);
			}
		}
		++attemptCount;
	}

	if(!connected) {
		std::cerr << "Cannot connect to Prepar3D" << std::endl;
		exit(1);
	}
}


Prepar3D::~Prepar3D(void)
{
	HRESULT hr = SimConnect_Close(hSimConnect); 
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
	case SIMCONNECT_RECV_ID_EXCEPTION: {
		SIMCONNECT_RECV_EXCEPTION *except = (SIMCONNECT_RECV_EXCEPTION*) pData;
		std::cerr << "Exception " << except->dwException << "\n";
		std::cerr << "ID " << except->dwID << "\n";
		std::cerr << "Index " << except->dwIndex << "\n";
		
		switch(except->dwException) {
			case SIMCONNECT_RECV_EXCEPTION::UNKNOWN_INDEX:
			std::cerr << "Unknown Index" << std::endl;
			break;

			case SIMCONNECT_RECV_EXCEPTION::UNKNOWN_SENDID:
			std::cerr << "Unknown sender ID" << std::endl;
			break;
		}

		break;
	}
		

	case SIMCONNECT_RECV_ID_OPEN:
		if(isVerbose()) {
			std::cout << "Open" << std::endl;
		}
		break;

    case SIMCONNECT_RECV_ID_EVENT:    { 
            SIMCONNECT_RECV_EVENT *evt = (SIMCONNECT_RECV_EVENT*)pData; 

            switch(evt->uEventID)  { 
                case EVENT_SIM_START: 
					if(isVerbose()) {
						std::cout << "Sim Start" << std::endl; 
					}
					
					// Defer creation of SimConnect requests until the sim proper has started.
					dataRequests.createRequests();
					started = true;

					if(isVerbose()) {
						std::cout << "Data Requests Created" << std::endl;
					}
                    break; 

                default: 
                   break; 
            } 
            break; 
        } 

        case SIMCONNECT_RECV_ID_SIMOBJECT_DATA: 
        { 
            SIMCONNECT_RECV_SIMOBJECT_DATA *pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA*)pData; 

			assert(pObjData->dwRequestID < dataRequests.size());
			SimObjectDataRequest* pRequest = dataRequests[pObjData->dwRequestID];

			if(pRequest != 0) {
				pRequest->handle(pObjData);
            } else  {
				std::cerr << "Unknown request ID: " <<  pObjData->dwRequestID << std::endl;
			}
            break; 
        } 

        case SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE: 
        { 
            SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE *pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE*)pData; 

			assert(pObjData->dwRequestID < dataRequests.size());
			SimObjectDataRequest* pRequest = dataRequests[pObjData->dwRequestID];

			if(pRequest != 0) {
				pRequest->handle(pObjData);
            } else  {
				std::cerr << "Unknown request ID: " <<  pObjData->dwRequestID << std::endl;
			}
            break; 
        } 


        case SIMCONNECT_RECV_ID_QUIT: 
        { 
            std::cout << "Sim quit" << std::endl;
            quit = true; 
            break; 
        } 

        default: 
            std::cout << "Received " << pData->dwID << std::endl; 
            break; 
    } 
}

size_t Prepar3D::registerDataRequest(SimObjectDataRequest* pRequest){
	assert(pRequest != 0);

	size_t id = dataRequests.add(pRequest);

	// If already started register (or re-register) any requests.
	if(started) {
		dataRequests.createRequests();
	}

	return id;		
}

void Prepar3D::unregisterDataRequest(SimObjectDataRequest* pRequest){
	assert(pRequest != 0);
	DWORD id = pRequest->getId();
	assert(id < dataRequests.size());
	dataRequests.remove(id);
}




