#include "stdafx.h"
#include <assert.h>
#include "Prepar3D.h"
#include "SimObjectDataRequest.h"


Prepar3D::Prepar3D(const char* appName)
	: hSimConnect(NULL),
	connected(false),
	quit(false)
{
	if(connected = SUCCEEDED(SimConnect_Open(&hSimConnect, appName, NULL, 0, 0, 0))) {
		printf("Connected\n");
        // Request an event when the simulation starts 
        HRESULT hr = SimConnect_SubscribeToSystemEvent(hSimConnect, EVENT_SIM_START, "SimStart"); 
	} else {
		printf("Cannot connect to Prepar3D\n");
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
	case SIMCONNECT_RECV_ID_EXCEPTION:
		printf("Exception\n");
		break;

	case SIMCONNECT_RECV_ID_OPEN:
		printf("Open\n");
		break;

    case SIMCONNECT_RECV_ID_EVENT:    { 
            SIMCONNECT_RECV_EVENT *evt = (SIMCONNECT_RECV_EVENT*)pData; 

            switch(evt->uEventID)  { 
                case EVENT_SIM_START: 
					// Defer creation of SimConnect requests until the sim proper has started.
					dataRequests.createRequests();
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
				printf("Unknown request ID:%u\n", pObjData->dwRequestID);
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
				printf("Unknown request ID:%u\n", pObjData->dwRequestID);
			}
            break; 
        } 


        case SIMCONNECT_RECV_ID_QUIT: 
        { 
            printf("\nSim quit\n"); 
            quit = true; 
            break; 
        } 

        default: 
            printf("\nReceived:%d",pData->dwID); 
            break; 
    } 
}

int Prepar3D::registerDataRequest(SimObjectDataRequest* pRequest){
	assert(pRequest != 0);

	int id = dataRequests.add(pRequest);
	return id;		
}

void Prepar3D::unregisterDataRequest(SimObjectDataRequest* pRequest){
	assert(pRequest != 0);
	DWORD id = pRequest->getId();
	assert(id < dataRequests.size());
	dataRequests.remove(id);
}




