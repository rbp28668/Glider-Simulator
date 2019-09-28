#pragma once
#include <map>
class SimObjectDataRequest;

// List of P3D data requests.
class SimObjectDataRequestList 
{
	std::map<long, SimObjectDataRequest*> requests;

public:

	SimObjectDataRequest* operator [](long idx);
	long add(SimObjectDataRequest* pItem, long idx);
	void remove(int idx);

	// Create SimConnect requests for each registered request.
	void createRequests();

	SimObjectDataRequestList();
	~SimObjectDataRequestList();
};

