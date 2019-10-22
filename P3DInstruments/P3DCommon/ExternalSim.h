#pragma once
#include <combaseapi.h> // for GUID
#include "SimObjectData.h"

//
// http://www.prepar3d.com/SDKv4/sdk/simconnect_api/references/simobject_functions.html#ExternalSimOverview
//


struct SimInputData {
	double latitude;
	double longitude;
	double altitude;
	double pitch;
	double bank;
	double heading;
	double groundAltitude;
	double staticCGToGround;
	int surfaceype;
};


struct SimOutputData {
	double latitude;	// radians
	double longitude;	// radians
	double altitude;	// feet
	double pitch;		// radians
	double bank;		// radians
	double heading;		// radians
	double vBodyX;
	double vBodyY;
	double vBodyZ;
	double vWorldX;
	double vWorldY;
	double vWorldZ;
	double verticalSpeed;
};

class ExternalSimVehicle;
class AIExternalSimVehicle;
class ExternalSim;

// Input data that knows to redirect any data requests into the ExternalSim object
class SimObjectInputData : public SimObjectData {
	ExternalSim* pExternalSim;
public:
	SimObjectInputData(Prepar3D* pTargetSim, DataItem* pItems, ExternalSim* pExternalSim);
	virtual void onData(void* pData, SimObject* pObject);
};

class ExternalSim
{
	Prepar3D* p3d;
	GUID guid;
	SimObjectInputData inputState;
	SimObjectData outputState;

	using Vehicles = std::map<DWORD, ExternalSimVehicle*>;
	Vehicles vehicles;

public:
	ExternalSim(Prepar3D* p3d);
	~ExternalSim();

#ifdef USE_EXTERNAL_SIM
	void create(SIMCONNECT_RECV_EXTERNAL_SIM_CREATE* pData);
	void destroy(SIMCONNECT_RECV_EXTERNAL_SIM_DESTROY* pData);
	void simulate(SIMCONNECT_RECV_EXTERNAL_SIM_SIMULATE* pData);
	void locationChanged(SIMCONNECT_RECV_EXTERNAL_SIM_LOCATION_CHANGED* pData);
	void event(SIMCONNECT_RECV_EXTERNAL_SIM_EVENT* pData);
#endif


	//Create - this is called when the object is created and we have the object ID.
	void create(ExternalSimVehicle* pVehicle);
	
	// Destroy - this is called whenever a SimObject using your External Sim is destroyed.
	void destroy(ExternalSimVehicle* pVehicle);
	
	//Simulate - this is called every simulation frame for each SimObject using your External Sim.
	void simulate(ExternalSimVehicle* pVehicle, SimInputData* pData);

	// Clears all the AI objects.
	void clear();


	void startCurrentVehicle(ExternalSimVehicle* pVehicle, const char* params);
	void startAIVehicle(AIExternalSimVehicle* pVehicle, const char* params);


};

