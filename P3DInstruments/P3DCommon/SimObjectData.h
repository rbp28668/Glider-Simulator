#pragma once
#include "Prepar3D.h"

// Manage a data definition used to communicate data to/from P3D.
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

protected:
	void show(void *pData);

public:
	virtual DataItem* items() = 0;
	virtual int itemCount() = 0;
	virtual void onData(void *pData) = 0;
	Prepar3D* getSim() { return pSim; }

public:
	SIMCONNECT_DATA_DEFINITION_ID getId() { return definitionId; }
	SimObjectData(Prepar3D* pTargetSim);
	virtual ~SimObjectData(void);

	void createDefinition();
	void handle(SIMCONNECT_RECV_SIMOBJECT_DATA *pObjData);

};

