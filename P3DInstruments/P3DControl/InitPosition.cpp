#include "stdafx.h"
#include <iostream>
#include "InitPosition.h"
#include <assert.h>

InitPosition::InitPosition(Prepar3D* pTargetSim)
	: SimObjectData(pTargetSim)
{
	assert(this);
	assert(pTargetSim);
}

void InitPosition::createDefinition()
{
	assert(this);
	HANDLE handle = getSim()->getHandle();
	SIMCONNECT_DATA_DEFINITION_ID definitionId = getId();

	HRESULT hr = ::SimConnect_AddToDataDefinition(
		handle
		, definitionId
		, "Initial Position"
		, ""   // don't put "NULL" whatever the documentation says!
		, SIMCONNECT_DATATYPE_INITPOSITION
		, 0
	);

	if (FAILED(hr)) {
		std::cerr << "Failed to create data definition for InitPosition" << std::endl;
	}
	else {
		if (getSim()->isVerbose()) {
			std::cout << "Created INITPOSITION data definition " << definitionId << std::endl;
			getSim()->showLastRequest("Add INITPOSITION Data to Data Definition");
		}
	}


}

void InitPosition::send(SIMCONNECT_DATA_INITPOSITION* pInit, SIMCONNECT_OBJECT_ID objectId)
{
	assert(this);
	assert(pInit);

	HANDLE handle = getSim()->getHandle();
	SIMCONNECT_DATA_DEFINITION_ID definitionId = getId();

	HRESULT hr = ::SimConnect_SetDataOnSimObject(
		handle, 
		definitionId, 
		objectId, 
		0, 
		0, 
		sizeof(SIMCONNECT_DATA_INITPOSITION), 
		pInit
	);
}
