#pragma once

#include "stdafx.h"
#include <SimConnect.h>
#include <vector>
#include "../P3DCommon/SimObjectDataRequestList.h"

class SimObjectDataRequest;

// Encapsulate the core P3D interface.
class Prepar3D
{
	bool connected;		// true if successfully connected.
	bool started;		// has received a start event.
	bool quit;			// set to true to terminate dispatch loop.
	bool verbose;		// set to true for verbose output.
	HANDLE hSimConnect; // use SimConnect_Open to set this value.

	SimObjectDataRequestList dataRequests; // keyed by request ID

    static void CALLBACK DispatchCallback(SIMCONNECT_RECV *pData, DWORD cbData, void *pContext);
    void Process(SIMCONNECT_RECV *pData, DWORD cbData);

	enum EVENT_ID{ 
		EVENT_SIM_START, 
	}; 

public:

	HANDLE getHandle() const { return hSimConnect; } 
	bool isVerbose() const { return verbose; } 
	void setVerbose(bool isVerbose) { verbose = isVerbose; }

    void Dispatch();
	void DispatchLoop();

	size_t registerDataRequest(SimObjectDataRequest* pRequest);
	void unregisterDataRequest(SimObjectDataRequest* pRequest);

	Prepar3D(const char* appName);
	virtual ~Prepar3D(void);
};

