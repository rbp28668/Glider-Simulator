#pragma once
#include "Prepar3D.h"

class SimObjectData
{
public:
	struct DataItem {
		const char*  DatumName;
		const char*  UnitsName;
		SIMCONNECT_DATATYPE  DatumType;
	};

private:
	Prepar3D* pSim;
	SIMCONNECT_DATA_DEFINITION_ID definitionId;
	static SIMCONNECT_DATA_DEFINITION_ID definitionIndex;

public:
	virtual DataItem* items() = 0;
	virtual int itemCount() = 0;
	virtual void onData(void *pData) = 0;

public:
	SIMCONNECT_DATA_DEFINITION_ID getId() { return definitionId; }
	SimObjectData(Prepar3D* pTargetSim);
	virtual ~SimObjectData(void);

	void createDefinition();
	void handle(SIMCONNECT_RECV_SIMOBJECT_DATA *pObjData);

};

