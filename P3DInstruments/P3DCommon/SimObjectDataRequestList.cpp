#include "StdAfx.h"
#include <iostream>
#include "SimObjectDataRequest.h"
#include "SimObjectDataRequestList.h"


SimObjectDataRequestList::SimObjectDataRequestList(void)
{
}


SimObjectDataRequestList::~SimObjectDataRequestList(void)
{
}

SimObjectDataRequest* SimObjectDataRequestList::operator[](long idx)
{
	SimObjectDataRequest* request = 0;
	std::map<long, SimObjectDataRequest*>::iterator it = requests.find(idx);
	if (it != requests.end()) {
		request = requests.at(idx);
	}
	return request;
}

long SimObjectDataRequestList::add(SimObjectDataRequest* pItem, long idx)
{
	requests[idx] = pItem;
	return idx;
}

void SimObjectDataRequestList::remove(int idx)
{
	requests.erase(idx);
}

void SimObjectDataRequestList::createRequests() {
	for(std::map<long, SimObjectDataRequest*>::iterator it = requests.begin();
		it != requests.end();
		++it) {
		SimObjectDataRequest* pRequest = it->second;
		if(pRequest != 0) {
			pRequest->createRequest();
			std::cout << "Data request " << it->first << " created" << std::endl;
		}
	}
}

