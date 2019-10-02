#pragma once
#include <map>
#include <list>
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>  // for DWORD
class SimObject;
class Prepar3D;

class SimObjectList
{
	std::map<DWORD, SimObject* > objects;
	std::list<SimObject*> unregisteredObjects;
	Prepar3D* p3d;
public:
	SimObjectList(Prepar3D* p3d);
	~SimObjectList();

	SimObject* newObject(DWORD dwRequestId);
	void associate(DWORD dwRequestId, DWORD dwObjectId);
};

