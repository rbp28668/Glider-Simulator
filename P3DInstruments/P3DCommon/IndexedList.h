#pragma once
#include <vector>
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

