#include "StdAfx.h"
#include<assert.h>
#include "SimObjectData.h"

// Sequence to allow each data definition to get its own unique index.
SIMCONNECT_DATA_DEFINITION_ID SimObjectData::definitionIndex = 0;

SimObjectData::SimObjectData(Prepar3D* pTargetSim)
	:pSim(pTargetSim)
	,definitionId(definitionIndex++)
{
	assert(pTargetSim != 0);
}

SimObjectData::~SimObjectData(void)
{
	::SimConnect_ClearDataDefinition(pSim->getHandle(), definitionId);
}

void SimObjectData::createDefinition() {
	int nItems = itemCount();
	for(int i=0; i<nItems; ++i) {
		DataItem* pItem = items() + i;
		HRESULT hr = ::SimConnect_AddToDataDefinition(pSim->getHandle(), definitionId, pItem->DatumName, pItem->UnitsName, pItem->DatumType); 
		if(FAILED(hr)) {
			printf("Failed to create data definition %s, units %s",pItem->DatumName, pItem->UnitsName);
		}
	}
}

void SimObjectData::handle(SIMCONNECT_RECV_SIMOBJECT_DATA *pObjData){
	assert(pObjData != 0);
	assert(pObjData->dwDefineID == definitionId);
	void *pData = (void*)&pObjData->dwData; 
	onData(pData);
}