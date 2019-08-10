#include "stdafx.h"
#include <assert.h>
#include "AIObject.h"
#include "../P3DCommon/Prepar3D.h"


enum DATA_REQUEST_ID {
	REQUEST_WHALE,
};


AIObject::AIObject(Prepar3D * p3d)
{
	assert(p3d);
	this->p3d = p3d;
}

AIObject::~AIObject()
{
}

void AIObject::createTestObject()
{
	// See http://www.prepar3d.com/SDKv4/sdk/simconnect_api/samples/ai_objects.html

	SIMCONNECT_DATA_INITPOSITION Init;
	HRESULT hr;

	Init.Altitude = 254;                // GRL altitude feet
	Init.Latitude = +(52 + (11.0 / 60) + (02.95 / 3600));        // 52 11 02.95N  (north +ve)
	Init.Longitude = -(000 + (06.0 / 60) + (30.95 / 3600));    // 000 06 30.95W  (west -ve
	Init.Pitch = 0.0;
	Init.Bank = 0.0;
	Init.Heading = 0.0;
	Init.OnGround = 1;
	Init.Airspeed = 0;
	// These have to be in SimObjects folder.  Look in .cfg file for name.
	//const char* objectName = "Humpbackwhale";
	//const char* objectName = "ANI_elephant_walk01_sm";
	//const char* objectName = "Lockheed Martin HC-130H - USCG";
	const char* objectName = "ASK 21 Mi D-KGSL";
	hr = SimConnect_AICreateSimulatedObject(p3d->getHandle(), objectName, Init, REQUEST_WHALE);

}

