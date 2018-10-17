#pragma once
#include "indexedlist.h"

class SimObjectDataRequest;

class SimObjectDataRequestList :
	public IndexedList
{
public:

	SimObjectDataRequest* operator [](int idx) { return (SimObjectDataRequest*)IndexedList::operator[](idx); }
	size_t add(SimObjectDataRequest* pItem) { return IndexedList::add(pItem); }
	//void remove(int idx) { return IndexedList::remove(idx); }
	// Create SimConnect requests for each registered request.
	void createRequests();

	SimObjectDataRequestList(void);
	~SimObjectDataRequestList(void);
};

