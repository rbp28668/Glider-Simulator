#pragma once
#include "stdafx.h"
#include <SimConnect.h>
#include "SimObject.h"

struct SimInputData;
struct SimOutputData;
class SimObjectData;
class SimObjectDataRequest;


// An external sim vehicle - use this for driving the main user sim object.
class ExternalSimVehicle: public SimObject
{
protected:
	const char* pszContainer;  // name of the object that's being simulated.
	SimObjectDataRequest* pInputRequest; // input request to get data to drive the object.
	LARGE_INTEGER t0;    // when this started.
	double frequency; // of timer in ticks per second.

public:

	double runTime() const;

	// Called to run one round of simulation
	virtual void simulate(const SimInputData& inputData, SimOutputData& outputData, double t) = 0;

	// Create the input request with the external sim input data.
	// This drives the external sim with a data packet per frame.
	void createInputRequest(SimObjectData* pData);
	
	// Type of aircraft this is simulating.
	const char* containerName();

	ExternalSimVehicle(Prepar3D* p3d, const char* containerName);
	virtual ~ExternalSimVehicle();
};

// An AI external sim vehicle.
class AIExternalSimVehicle : public ExternalSimVehicle {

public:

	virtual SIMCONNECT_DATA_INITPOSITION initialPosition();
	virtual void onCreate();
	AIExternalSimVehicle(Prepar3D* p3d, const char* containerName);
	virtual ~AIExternalSimVehicle();

};
