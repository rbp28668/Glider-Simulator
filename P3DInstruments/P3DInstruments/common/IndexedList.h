#pragma once
#include <vector>
class IndexedList
{

	std::vector<void*> items; // keyed by request ID

protected:

	void* operator [](int idx);
	int add(void* pItem);
	void remove(int idx);
	int size() {return items.size();}

	IndexedList(void);
	~IndexedList(void);
};

