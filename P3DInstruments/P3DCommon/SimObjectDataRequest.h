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
	SimObjectData* pData;

	SIMCONNECT_DATA_REQUEST_ID requestId;

	static SIMCONNECT_DATA_REQUEST_ID requestIndex;

public:

	SIMCONNECT_DATA_REQUEST_ID getId() { return requestId; }

	void setPeriod(SIMCONNECT_PERIOD period) { this->period = period; }
	void setOrigin(DWORD origin) { this->origin = origin; }
	void setInterval(DWORD interval) { this->interval = interval; }
    void setLimit(DWORD limit) { this->limit = limit; }

	SimObjectDataRequest(Prepar3D* pSim, SimObjectData* pData);
	virtual ~SimObjectDataRequest(void);

	void createRequest(void);
	void handle(SIMCONNECT_RECV_SIMOBJECT_DATA *pObjData);
};

