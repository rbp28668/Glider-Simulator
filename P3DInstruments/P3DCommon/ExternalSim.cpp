#include "stdafx.h"
#include <assert.h>
#include "Prepar3D.h"
#include "SimObjectData.h"
#include "SimObjectDataRequest.h"
#include "ExternalSim.h"
#include "ExternalSimVehicle.h"


// Data from the sim, Input to the simulation aircraft
static SimObjectData::DataItem InputStateItems[] = {
	{"PLANE LATITUDE", "radians", SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE LONGITUDE", "radians", SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE ALTITUDE", "feet", SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE PITCH DEGREES", "radians", SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE BANK DEGREES", "radians", SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE HEADING DEGREES TRUE", "radians", SIMCONNECT_DATATYPE_FLOAT64},
	{"GROUND ALTITUDE", "meters", SIMCONNECT_DATATYPE_FLOAT64},
	{"STATIC CG TO GROUND", "feet", SIMCONNECT_DATATYPE_FLOAT64},
	{"SURFACE TYPE", "enum", SIMCONNECT_DATATYPE_INT32},
	{0,0,SIMCONNECT_DATATYPE_INVALID}
};


// Data back to the sim, Output from the simulation aircraft
static SimObjectData::DataItem OutputStateItems[] = {
	{"PLANE LATITUDE", "radians", SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE LONGITUDE", "radians", SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE ALTITUDE", "feet", SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE PITCH DEGREES", "radians", SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE BANK DEGREES", "radians", SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE HEADING DEGREES TRUE", "radians", SIMCONNECT_DATATYPE_FLOAT64},
	{"VELOCITY BODY X", "m/s", SIMCONNECT_DATATYPE_FLOAT64},
	{"VELOCITY BODY Y", "m/s", SIMCONNECT_DATATYPE_FLOAT64},
	{"VELOCITY BODY Z", "m/s", SIMCONNECT_DATATYPE_FLOAT64},
	{"VELOCITY WORLD X", "m/s", SIMCONNECT_DATATYPE_FLOAT64},
	{"VELOCITY WORLD Y", "m/s", SIMCONNECT_DATATYPE_FLOAT64},
	{"VELOCITY WORLD Z", "m/s", SIMCONNECT_DATATYPE_FLOAT64},
	{"VERTICAL SPEED", "m/s", SIMCONNECT_DATATYPE_FLOAT64},
	{0,0,SIMCONNECT_DATATYPE_INVALID}
};

// Data request to drive the sim if we're not *actually* using external sim.
SimObjectInputData::SimObjectInputData(Prepar3D* pTargetSim, DataItem* pItems, ExternalSim* pExternalSim)
	: SimObjectData(pTargetSim, pItems)
	, pExternalSim(pExternalSim)
{
}

// Called when we have an input data packet to drive any of the external objects.
void SimObjectInputData::onData(void* pData, SimObject* pObject)
{
	ExternalSimVehicle* pVehicle = static_cast<ExternalSimVehicle*>(pObject);
	SimInputData* pSimInput = static_cast<SimInputData*>(pData);
	pExternalSim->simulate(pVehicle, pSimInput);
}


ExternalSim::ExternalSim(Prepar3D* p3d)
	: p3d(p3d)
	, inputState(p3d, InputStateItems, this)
	, outputState(p3d, OutputStateItems)
{
	HRESULT hr = CoCreateGuid(&guid);
	inputState.createDefinition();
	outputState.createDefinition();

#ifdef USE_EXTERNAL_SIM

	SIMCONNECT_EXTERNAL_SIM_CALLBACK_FLAG  callbackMask =
		SIMCONNECT_EXTERNAL_SIM_CALLBACK_FLAG_CREATE |
		SIMCONNECT_EXTERNAL_SIM_CALLBACK_FLAG_DESTROY |
		SIMCONNECT_EXTERNAL_SIM_CALLBACK_FLAG_SIMULATE |
		SIMCONNECT_EXTERNAL_SIM_CALLBACK_FLAG_LOCATION_CHANGED |
		SIMCONNECT_EXTERNAL_SIM_CALLBACK_FLAG_EVENT;

	hr = ::SimConnect_RegisterExternalSim(
		p3d->getHandle(),
		guid,
		callbackMask,
		inputState.getId()
	);
#endif
}

ExternalSim::~ExternalSim()
{
#ifdef USE_EXTERNAL_SIM
	HRESULT hr = ::SimConnect_UnregisterExternalSim(
		p3d->getHandle(),
		guid
	);
#endif
}

#ifdef USE_EXTERNAL_SIM
void ExternalSim::create(SIMCONNECT_RECV_EXTERNAL_SIM_CREATE* pData)
{
	pData->dwObjectID;
}

void ExternalSim::destroy(SIMCONNECT_RECV_EXTERNAL_SIM_DESTROY* pData)
{
}

void ExternalSim::simulate(SIMCONNECT_RECV_EXTERNAL_SIM_SIMULATE* pData)
{
	if (pData->bShouldSimulate) {
		SimInputData* pInput = reinterpret_cast<SimInputData*>(&(pData->dwData));
		ExternalSimVehicle* pVehicle = vehicles[pData->dwObjectID];
		SimOutputData outputData;
		if (pVehicle->simulate(*pInput, outputData, pVehicle->runTime())) {
			::SimConnect_SetDataOnSimObject(
				p3d->getHandle(),
				outputState.getId(),
				pVehicle->id(),
				0,
				0,
				sizeof(SimOutputData),
				&outputData
			);
		}
	}
}

void ExternalSim::locationChanged(SIMCONNECT_RECV_EXTERNAL_SIM_LOCATION_CHANGED* pData)
{
}

void ExternalSim::event(SIMCONNECT_RECV_EXTERNAL_SIM_EVENT* pData)
{
}
#endif

// Called by the vehicle when P3D has created the actual
// vehicle and it has an object ref.
void ExternalSim::create(ExternalSimVehicle* pVehicle)
{
	assert(this);
	assert(pVehicle);
	vehicles[pVehicle->id()] = pVehicle;
	pVehicle->createInputRequest(&inputState);
}

// Called when deleting an external sim vehicle to tear down
// any references to it.
void ExternalSim::destroy(ExternalSimVehicle* pVehicle)
{
	vehicles.erase(pVehicle->id());
}

// Call when we get a data packet to drive the sim - whether from a normal
// repeating data request of whether from the external sim.
void ExternalSim::simulate(ExternalSimVehicle* pVehicle, SimInputData*pData)
{

	SimOutputData outputData;
	if (pVehicle->simulate(*pData, outputData, pVehicle->runTime())) {
		::SimConnect_SetDataOnSimObject(
			p3d->getHandle(),
			outputState.getId(),
			pVehicle->id(),
			0,
			0,
			sizeof(SimOutputData),
			&outputData
		);
	}
	else { // finished
		if (pVehicle->restartOnFinish()) {
			pVehicle->restart();
		}
		else {
			delete pVehicle;  // will call destroy etc.
		}
	}
	
}

// Clears out all the AI objects.  They're deleted and should disappear.
void ExternalSim::clear()
{
	// user aircraft may be in operation, remove if so.
	ExternalSimVehicle* user = 0;
	auto iter = vehicles.find(p3d->userAircraft().id());
	if (iter != vehicles.end()) {
		user = iter->second;
		vehicles.erase(iter);
	}

	// All the rest must be sim objects so....
	while(!vehicles.empty())
	{
		iter = vehicles.begin();
		ExternalSimVehicle* pVehicle = iter->second;
		iter->second = 0;
		delete pVehicle;
	}

	// Put back the user aircraft in necessary.
	if (user) {
		vehicles[user->id()] = user;
	}

}

// Start the user's vehicle with the given ExternalSimVehicle.
void ExternalSim::startCurrentVehicle(ExternalSimVehicle* pVehicle, const char* params)
{

#ifdef USE_EXTERNAL_SIM
	HRESULT hr = ::SimConnect_ChangeVehicleWithExternalSim(
		p3d->getHandle(),
		pVehicle->containerName(),
		guid,
		params,
		13
	);

	vehicles[SIMCONNECT_OBJECT_ID_USER] = pVehicle;
#else
	HRESULT hr = ::SimConnect_ChangeVehicle(
		p3d->getHandle(),
		pVehicle->containerName()
		);

	// TODO, want to FREEZE_LATITUDE_LONGITUDE_SET,  FREEZE_ALTITUDE_SET and  FREEZE_ATTITUDE_SET
	// So that P3d isn't trying to drive the vehicle.

	// Vehicle isn't created in the same way as an external AI vehicle so call create
	// "by hand" to ensure vehicle is registered, will get input data, events etc.
	create(pVehicle);

#endif

}

// Start an AI vehicle.
void ExternalSim::startAIVehicle(AIExternalSimVehicle* pVehicle, const char* params)
{
	DWORD dwRequestId = p3d->nextRequestId();
	p3d->registerSimObject(pVehicle, dwRequestId);  // so we pick up the objectID when created.

#ifdef USE_EXTERNAL_SIM


	HRESULT hr = ::SimConnect_AICreateObjectWithExternalSim(
		p3d->getHandle(),
		pVehicle->containerName(),
		pVehicle->initialPosition(),
		dwRequestId,
		guid,
		params,
		0
	);
	
#else
	HRESULT hr = ::SimConnect_AICreateSimulatedObject(
		p3d->getHandle(), 
		pVehicle->containerName(), 
		pVehicle->initialPosition(), 
		dwRequestId
	);
#endif
}

