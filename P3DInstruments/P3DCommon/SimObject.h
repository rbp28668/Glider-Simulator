#pragma once
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>  // for DWORD

class Prepar3D;

// Class to keep track of a sim object created by the client code.
class SimObject
{
	DWORD objectId;
	DWORD requestId;
	const char* pszName;

protected:
	Prepar3D* p3d; // need ref as destructor needs to clean up.

public:
	SimObject(Prepar3D* p3d, DWORD requestId);
	SimObject(Prepar3D* p3d);
	virtual void onCreate();
	virtual ~SimObject();

	void setName(const char* pszName) { this->pszName = pszName; }
	void setObjectId(DWORD objectId);
	void setRequestId(DWORD requestId);
	DWORD id() const {return objectId; }
	DWORD request() const { return requestId; }

};

