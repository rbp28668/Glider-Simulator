#include "stdafx.h"
#include "Thread.h"
#include <process.h>    /* _beginthread, _endthread */


Thread::Thread(bool autostart)
{
	if (autostart) {
		start();
	}
}


Thread::~Thread()
{
	::CloseHandle((HANDLE)threadHandle);
}

void Thread::start()
{
	::_beginthreadex(0, 0, &threadFunction, this, 0, &thrdaddr);
}

void Thread::waitToFinish()
{
	::WaitForSingleObject((HANDLE)threadHandle, INFINITE);
}

unsigned __stdcall Thread::threadFunction(void * obj) {
	Thread* pThread = static_cast<Thread*>(obj);
	unsigned ret =  pThread->run();
	_endthreadex(0);
	return ret;
}

