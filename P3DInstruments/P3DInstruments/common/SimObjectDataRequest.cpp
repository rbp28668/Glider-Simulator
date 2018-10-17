#include "StdAfx.h"
#include <assert.h>
#include "SimObjectDataRequest.h"


// Create default request for Sim Frame rate notifications
SimObjectDataRequest::SimObjectDataRequest(Prepar3D* pTargetSim, SimObjectData* pData)
	: pSim(pTargetSim)
	, pData(pData)
	, period(SIMCONNECT_PERIOD_SIM_FRAME)
	, origin(0)
	, interval(0)
	, limit(0)
	
{
	assert(pTargetSim != 0);
	assert(pData != 0);

	requestId = pSim->registerDataRequest(this);
}


SimObjectDataRequest::~SimObjectDataRequest(void)
{
	if(pData != 0) {
		HRESULT hr = ::SimConnect_RequestDataOnSimObject(pSim->getHandle(),  requestId,  pData->getId(),  SIMCONNECT_OBJECT_ID_USER,
			SIMCONNECT_PERIOD_NEVER, 0, 0, 0, 0 );
	}

    pSim->unregisterDataRequest(this);

}


void SimObjectDataRequest::createRequest( void) {
 
  HRESULT hr = ::SimConnect_RequestDataOnSimObject(pSim->getHandle(),  requestId,  pData->getId(),  SIMCONNECT_OBJECT_ID_USER,
	period,	0, origin, interval, limit );
}


void SimObjectDataRequest::	handle(SIMCONNECT_RECV_SIMOBJECT_DATA *pObjData) {
	assert(pObjData != 0);
	assert(pObjData->dwID == SIMCONNECT_RECV_ID_SIMOBJECT_DATA);
	assert(pObjData->dwRequestID == requestId); // confirm it really is this request.
	assert(pData != 0); // shouldn't get a callback before pData is set up.
	pData->handle(pObjData);
}
