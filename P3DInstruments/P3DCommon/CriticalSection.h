#pragma once
class CriticalSection
{
	CRITICAL_SECTION cs;

public:
	class Lock {
		CRITICAL_SECTION* pcs;
	public:
		Lock(CriticalSection& cs);
		~Lock();
	};

	CriticalSection();
	~CriticalSection();
};

