#include "StdAfx.h"
#include <assert.h>
#include "IndexedList.h"


IndexedList::IndexedList(void)
{
}


IndexedList::~IndexedList(void)
{
}

void* IndexedList::operator [](int idx){
	assert(idx >= 0);
	assert(idx < items.size());
	return items[idx];
}

size_t IndexedList::add(void* pItem){
	assert(pItem != 0);

	for(size_t i=0; i<items.size(); ++i) {
		if( items[i] == 0) {
			items[i] = pItem;
			return i;
		}
	}
	items.push_back(pItem);
	return items.size()-1;
}

void IndexedList::remove(int idx){
	assert(idx >= 0);
	assert(idx < items.size());
	assert(items[idx] != 0);
	items[idx] = 0;
}

