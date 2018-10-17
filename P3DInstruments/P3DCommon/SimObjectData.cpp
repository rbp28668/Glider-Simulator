#include "StdAfx.h"
#include<assert.h>
#include <iostream>
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
		if(pSim->isVerbose()) {
			std::cout << "Creating data definition " << definitionId << " adding " << pItem->DatumName << " units " << pItem->UnitsName << std::endl;
		}

		HRESULT hr = ::SimConnect_AddToDataDefinition(pSim->getHandle(), definitionId, pItem->DatumName, pItem->UnitsName, pItem->DatumType); 
		if(FAILED(hr)) {
			std::cerr << "Failed to create data definition " << pItem->DatumName << " units " << pItem->UnitsName << std::endl;
		}
	}
}

void SimObjectData::handle(SIMCONNECT_RECV_SIMOBJECT_DATA *pObjData){
	assert(pObjData != 0);
	assert(pObjData->dwDefineID == definitionId);
	void *pData = (void*)&pObjData->dwData; 
	onData(pData);
}

void SimObjectData::show(void* pData) {
	int nItems = itemCount();
	SimObjectData::DataItem* pItem = items();
	
	printf("-------------------------------------\n");
	printf("double %Iu, long %Iu\n",sizeof(double), sizeof(__int64));
	for(int i=0; i<nItems; ++i, ++pItem) {
		switch(pItem->DatumType) {
		case SIMCONNECT_DATATYPE_FLOAT64:
			printf("%s: %f %s  \t\t(%8.8I64x,%8.8I64x)\n",pItem->DatumName, *(double*)pData, pItem->UnitsName, *(__int64*)pData >> 32, *(__int64*)pData);
			pData = (char*)pData + sizeof(double);
			break;
		case SIMCONNECT_DATATYPE_INT64:
			printf("%s: %16.16I64x %s\n",pItem->DatumName, *(__int64*)pData, pItem->UnitsName);
			pData = (char*)pData + sizeof(__int64);
			break;
		default:
			printf("%s unhandled type \n", pItem->DatumName);
			break;
		}
	}
}
