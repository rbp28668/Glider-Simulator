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
	DataItem* pdi;
	int count;

protected:
	void show(void *pData);

public:
	virtual DataItem* items();  // over-ride as necessary to reference a set of items
	virtual int itemCount();    // over-ride as necessary to provide count for above.
	virtual void onData(void* pData, SimObject* pObject); // no-op by default, override as necessary
	Prepar3D* getSim() { return pSim; }

public:
	SIMCONNECT_DATA_DEFINITION_ID getId() { return definitionId; }
	SimObjectData(Prepar3D* pTargetSim);
	SimObjectData(Prepar3D* pTargetSim, DataItem* pItems);
	virtual ~SimObjectData(void);

	void createDefinition();
	void handle(SIMCONNECT_RECV_SIMOBJECT_DATA *pObjData, SimObject* pObject);
	void send(void* pObjData, size_t dataSize, SIMCONNECT_OBJECT_ID objectID = SIMCONNECT_OBJECT_ID_USER);

};

