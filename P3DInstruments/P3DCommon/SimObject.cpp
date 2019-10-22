#include "stdafx.h"
#include "SimObject.h"
#include "..//P3DCommon/Prepar3D.h"

SimObject::SimObject(Prepar3D* p3d, DWORD requestId)
	: p3d(p3d)
	, requestId(requestId)
	, objectId((DWORD)-1)
{
}

SimObject::SimObject(Prepar3D* p3d)
	: p3d(p3d)
	, requestId(-1)
	, objectId((DWORD)-1)
{
}

void SimObject::onCreate()
{
	// Default behaviour is no operation.
}

SimObject::~SimObject()
{
	// Kill off in P3D first so we don't get events for dead objects.
	if (objectId != SIMCONNECT_OBJECT_ID_USER) {
		::SimConnect_AIRemoveObject(p3d->getHandle(), objectId, p3d->nextRequestId());
		p3d->unregisterSimObject(objectId);
	}

	p3d = 0;
}

void SimObject::setObjectId(DWORD objectId)
{
	this->objectId = objectId;
}

void SimObject::setRequestId(DWORD requestId)
{ 
	this->requestId = requestId;
}
