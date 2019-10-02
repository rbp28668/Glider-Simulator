#pragma once
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>  // for DWORD

class Prepar3D;

class SimObject
{
	DWORD objectId;
	DWORD requestId;
	Prepar3D* p3d; // need ref as destructor needs to clean up.

public:
	SimObject(Prepar3D* p3d, DWORD requestId);
	~SimObject();


	void setObjectId(DWORD objectId);
	DWORD id() const {return objectId; }
	DWORD request() const { return requestId; }

};

