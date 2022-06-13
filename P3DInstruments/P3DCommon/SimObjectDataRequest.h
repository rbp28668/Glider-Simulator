#pragma once
#include "Prepar3D.h"
#include "SimObjectData.h"

// Manage a data request from P3D.
class SimObjectDataRequest
{

private:

	SIMCONNECT_PERIOD period;
	DWORD origin;
	DWORD interval;
	DWORD limit;
	
	Prepar3D* pSim;
	SimObjectData* pData;   // Request this data
	SimObject* pSimObject;  // for this object

	SIMCONNECT_DATA_REQUEST_ID requestId;

	static SIMCONNECT_DATA_REQUEST_ID requestIndex;

public:

	SIMCONNECT_DATA_REQUEST_ID getId() { return requestId; }

	SimObjectDataRequest(Prepar3D* pSim, SimObjectData* pData, SimObject* pSimObject, SIMCONNECT_PERIOD period = SIMCONNECT_PERIOD_VISUAL_FRAME, DWORD origin = 0, DWORD interval = 0, DWORD limit = 0);
	virtual ~SimObjectDataRequest(void);

	void createRequest(void);
	void handle(SIMCONNECT_RECV_SIMOBJECT_DATA *pObjData);
	void stop();
};

