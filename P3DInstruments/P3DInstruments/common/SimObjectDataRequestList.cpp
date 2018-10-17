#include "StdAfx.h"
#include "SimObjectDataRequest.h"
#include "SimObjectDataRequestList.h"


SimObjectDataRequestList::SimObjectDataRequestList(void)
{
}


SimObjectDataRequestList::~SimObjectDataRequestList(void)
{
}

void SimObjectDataRequestList::createRequests() {
	for(int i=0; i<size(); ++i) {
		SimObjectDataRequest* pRequest = operator[](i);
		if(pRequest != 0) {
			pRequest->createRequest();
		}
	}
}

