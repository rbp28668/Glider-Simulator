#pragma once
#include <vector>

// Class to store a list of items to allow them to be referenced by ID.
// order is effectively random as it will reuse freed slots.
class IndexedList
{

	std::vector<void*> items; // keyed by request ID

protected:

	void* operator [](int idx);
	size_t add(void* pItem);

public:
	void remove(int idx);
	size_t size() {return items.size();}

protected:
	IndexedList(void);
	~IndexedList(void);
};

