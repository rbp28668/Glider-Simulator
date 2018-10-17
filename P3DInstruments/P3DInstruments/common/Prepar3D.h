#pragma once

#include "stdafx.h"
#include <vector>
#include "SimObjectDataRequestList.h"

class SimObjectDataRequest;

class Prepar3D
{
	bool connected;		// true if successfully connected.
	bool quit;			// set to true to terminate dispatch loop.
	HANDLE hSimConnect; // use SimConnect_Open to set this value.

	SimObjectDataRequestList dataRequests; // keyed by request ID

    static void CALLBACK DispatchCallback(SIMCONNECT_RECV *pData, DWORD cbData, void *pContext);
    void Process(SIMCONNECT_RECV *pData, DWORD cbData);

	static enum EVENT_ID{ 
		EVENT_SIM_START, 
	}; 

public:

	HANDLE getHandle() { return hSimConnect; }

    void Dispatch();
	void DispatchLoop();

	int registerDataRequest(SimObjectDataRequest* pRequest);
	void unregisterDataRequest(SimObjectDataRequest* pRequest);

	Prepar3D(const char* appName);
	virtual ~Prepar3D(void);
};

