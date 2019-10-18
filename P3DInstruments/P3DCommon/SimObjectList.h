#pragma once
#include <map>
#include <list>
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>  // for DWORD
class SimObject;
class Prepar3D;

class SimObjectList
{
	using Objects = std::map<DWORD, SimObject* >;
	Objects objects;
	std::list<SimObject*> unregisteredObjects;
	Prepar3D* p3d;
public:
	SimObjectList(Prepar3D* p3d);
	~SimObjectList();

	SimObject* newObject(DWORD dwRequestId);
	void add(SimObject* pObject, DWORD dwRequestId);
	void associate(DWORD dwRequestId, DWORD dwObjectId);
	SimObject* lookup(DWORD dwObjectId);
	void remove(DWORD dwObjectId);

};

