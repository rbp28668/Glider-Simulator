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


ExternalSim::ExternalSim(Prepar3D* p3d)
	: p3d(p3d)
	, inputState(p3d, InputStateItems, this)
	, outputState(p3d, OutputStateItems)
{
	HRESULT hr = CoCreateGuid(&guid);
	inputState.createDefinition();
	outputState.createDefinition();

	
}

ExternalSim::~ExternalSim()
{
}


void ExternalSim::create(ExternalSimVehicle* pVehicle)
{
	assert(this);
	assert(pVehicle);
	vehicles[pVehicle->id()] = pVehicle;
	pVehicle->createInputRequest(&inputState);
}

void ExternalSim::destroy(ExternalSimVehicle* pVehicle)
{
	vehicles.erase(pVehicle->id());
}

void ExternalSim::simulate(ExternalSimVehicle* pVehicle, SimInputData*pData)
{

	SimOutputData outputData;
	pVehicle->simulate(*pData, outputData, pVehicle->runTime()); 
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


void ExternalSim::startCurrentVehicle(ExternalSimVehicle* pVehicle, const char* params)
{
	HRESULT hr = ::SimConnect_ChangeVehicleWithExternalSim(
		p3d->getHandle(),
		pVehicle->containerName(),
		guid,
		params,
		13
	);

	vehicles[SIMCONNECT_OBJECT_ID_USER] = pVehicle;
}

void ExternalSim::startAIVehicle(AIExternalSimVehicle* pVehicle, const char* params)
{
	DWORD dwRequestId = p3d->nextRequestId();
	p3d->registerSimObject(pVehicle, dwRequestId);  // so we pick up the objectID when created.

	HRESULT hr = SimConnect_AICreateSimulatedObject(p3d->getHandle(), pVehicle->containerName(), pVehicle->initialPosition(), dwRequestId);
}

SimObjectInputData::SimObjectInputData(Prepar3D* pTargetSim, DataItem* pItems,ExternalSim* pExternalSim)
	: SimObjectData(pTargetSim, pItems)
	, pExternalSim(pExternalSim)
{
}

void SimObjectInputData::onData(void* pData, SimObject* pObject)
{
	ExternalSimVehicle* pVehicle = static_cast<ExternalSimVehicle*>(pObject);
	SimInputData* pSimInput = static_cast<SimInputData*>(pData);
	pExternalSim->simulate(pVehicle, pSimInput);
}
