#include "stdafx.h"
#include <iostream>
#include "CriticalSection.h"


CriticalSection::CriticalSection()
{
	::InitializeCriticalSection(&cs);
}

CriticalSection::~CriticalSection()
{
	::DeleteCriticalSection(&cs);
}

CriticalSection::Lock::Lock(CriticalSection& cs) : pcs(&(cs.cs))
{
	//std::cout << "Entering CS" << std::endl;
	::EnterCriticalSection(pcs);
	//std::cout << "In CS" << std::endl;
}

CriticalSection::Lock::~Lock()
{
	//std::cout << "Leaving CS" << std::endl;
	::LeaveCriticalSection(pcs);
	//std::cout << "Left CS" << std::endl;

}
