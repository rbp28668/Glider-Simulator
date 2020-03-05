#pragma once
#include "../P3DCommon/SimObjectData.h"
class InitPosition :
	public SimObjectData
{
public:
	InitPosition(Prepar3D* pTargetSim);
	void createDefinition();
	void send(SIMCONNECT_DATA_INITPOSITION* pInit, SIMCONNECT_OBJECT_ID objectId);
};

