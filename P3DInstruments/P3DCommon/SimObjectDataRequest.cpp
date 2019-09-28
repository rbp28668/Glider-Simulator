#include "StdAfx.h"
#include <iostream>
#include <assert.h>
#include "SimObjectDataRequest.h"


// Create default request for data notifications.  Note that the handling of data is delegated to the SimObjectData
// when data is received from SimConnect via the dispatch loop.
SimObjectDataRequest::SimObjectDataRequest(Prepar3D* pTargetSim, SimObjectData* pData, SIMCONNECT_PERIOD period)
	: pSim(pTargetSim)
	, pData(pData)
	, period(period)
	, origin(0)
	, interval(0)
	, limit(0)
	
{
	assert(pTargetSim != 0);
	assert(pData != 0);

	requestId = pSim->registerDataRequest(this);
	if(pTargetSim->isVerbose()) {
		std::cout << "Created data request with ID " << requestId << std::endl;
	}
}


SimObjectDataRequest::~SimObjectDataRequest(void)
{
	if(pData != 0) {
		HRESULT hr = ::SimConnect_RequestDataOnSimObject(pSim->getHandle(),  requestId,  pData->getId(),  SIMCONNECT_OBJECT_ID_USER,
			SIMCONNECT_PERIOD_NEVER, 0, 0, 0, 0 );
		if(FAILED(hr)) {
			std::cerr << "Failed to unregister data request " << requestId << std::endl;
		}

	}

    pSim->unregisterDataRequest(this);

}


void SimObjectDataRequest::createRequest( void) {
 
  HRESULT hr = ::SimConnect_RequestDataOnSimObject(pSim->getHandle(),  requestId,  pData->getId(),  SIMCONNECT_OBJECT_ID_USER,
	period,	0, origin, interval, limit );
  if(SUCCEEDED(hr)) {
	  if(pSim->isVerbose()) {
		  std::cout << "Requested data for ID " << requestId << " with data ID " << pData->getId() << std::endl;
		  pSim->showLastRequest("Requested data on sim object ");
	  }
  } else {
	  std::cerr << "Unable to create data request " << requestId << " for data ID " << pData->getId() << std::endl;
  }
}


void SimObjectDataRequest::	handle(SIMCONNECT_RECV_SIMOBJECT_DATA *pObjData) {
	assert(pObjData != 0);
	assert(pObjData->dwID == SIMCONNECT_RECV_ID_SIMOBJECT_DATA);
	assert(pObjData->dwRequestID == requestId); // confirm it really is this request.
	assert(pData != 0); // shouldn't get a callback before pData is set up.
	pData->handle(pObjData);
}
