#include "stdafx.h"
#include "ExternalSimVehicle.h"
#include "Prepar3D.h"
#include "ExternalSim.h"
#include "SimObjectDataRequest.h"




double ExternalSimVehicle::runTime() const
{
	LARGE_INTEGER now;
	::QueryPerformanceCounter(&now);
	double dt = (double)(now.QuadPart - t0.QuadPart);
	return dt / frequency;
}

void ExternalSimVehicle::createInputRequest(SimObjectData* pData)
{
	SimObjectDataRequest* pInputRequest = new SimObjectDataRequest(p3d, pData, this, SIMCONNECT_PERIOD_VISUAL_FRAME);
	// TODO SAVE INPUT REQUEST
}

const char* ExternalSimVehicle::containerName()
{
	return pszContainer;
}

ExternalSimVehicle::ExternalSimVehicle(Prepar3D* p3d, const char* containerName)
	: pszContainer(containerName)
	, SimObject(p3d)
	, pInputRequest(0)
{
	setName(containerName);

	LARGE_INTEGER freq;
	::QueryPerformanceFrequency(&freq);
	frequency = (double)freq.QuadPart;

	::QueryPerformanceCounter(&t0);

}

ExternalSimVehicle::~ExternalSimVehicle()
{
	if (pInputRequest) {
		delete pInputRequest;
	}
}


SIMCONNECT_DATA_INITPOSITION AIExternalSimVehicle::initialPosition()
{
	return SIMCONNECT_DATA_INITPOSITION();
}

// Called when the object ID has been allocated.  
void AIExternalSimVehicle::onCreate()
{
	// Want to remove control from the AI (and destroy the AI)
	// as we're taking complete control.
	HRESULT hr = ::SimConnect_AIReleaseControlEx(
		p3d->getHandle(),
		this->id(),
		p3d->nextRequestId(),
		TRUE   // destroy the AI as not needed
	);

	// Update T0 time 
	::QueryPerformanceCounter(&t0);

	p3d->externalSim().create(this);

}

AIExternalSimVehicle::AIExternalSimVehicle(Prepar3D* p3d, const char* containerName)
	: ExternalSimVehicle(p3d, containerName)

{
}

AIExternalSimVehicle::~AIExternalSimVehicle()
{
}

