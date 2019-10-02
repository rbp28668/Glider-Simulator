#include "stdafx.h"
#include "SimObject.h"
#include "SimObjectList.h"

SimObjectList::SimObjectList(Prepar3D* p3d) : p3d(p3d)
{
}

SimObjectList::~SimObjectList()
{
	for (std::map<DWORD, SimObject*>::iterator it = objects.begin();
		it != objects.end();
		++it) {
		delete it->second;
		it->second = 0;
	}
}

SimObject* SimObjectList::newObject(DWORD dwRequestId)
{
	SimObject* pObj = new SimObject(p3d, dwRequestId);
	unregisteredObjects.push_back(pObj);
	return pObj;
}

void SimObjectList::associate(DWORD dwRequestId, DWORD dwObjectId)
{
	SimObject* pObject = 0;
	for (std::list<SimObject*>::iterator it = unregisteredObjects.begin();
		it != unregisteredObjects.end();
		++it) {
		if ((*it)->request() == dwRequestId) {
			pObject = (*it);
			pObject->setObjectId(dwObjectId);
			objects[dwObjectId] = pObject; // save for later lookup

			// Here is a good place to do anything that needed to wait for
			// p3d to provide the object Id (like associate a flight plan).
		}
	}
	unregisteredObjects.remove(pObject);
}
