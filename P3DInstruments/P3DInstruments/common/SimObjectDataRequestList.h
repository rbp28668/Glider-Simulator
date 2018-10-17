#pragma once
#include "indexedlist.h"

class SimObjectDataRequest;

class SimObjectDataRequestList :
	private IndexedList
{
public:

	SimObjectDataRequest* operator [](int idx) { return (SimObjectDataRequest*)IndexedList::operator[](idx); }
	int add(SimObjectDataRequest* pItem) { return IndexedList::add(pItem); }
	void remove(int idx) { return IndexedList::remove(idx); }

	// Create SimConnect requests for each registered request.
	void createRequests();

	SimObjectDataRequestList(void);
	~SimObjectDataRequestList(void);
};

