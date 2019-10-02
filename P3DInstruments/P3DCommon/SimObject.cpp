#include "stdafx.h"
#include "SimObject.h"
#include "..//P3DCommon/Prepar3D.h"

SimObject::SimObject(Prepar3D* p3d, DWORD requestId)
	: p3d(p3d)
	, requestId(requestId)
	, objectId((DWORD)-1)
{
}

SimObject::~SimObject()
{
	::SimConnect_AIRemoveObject(p3d->getHandle(), objectId, p3d->nextRequestId());
	p3d = 0;
}

void SimObject::setObjectId(DWORD objectId)
{
	this->objectId = objectId;
}
