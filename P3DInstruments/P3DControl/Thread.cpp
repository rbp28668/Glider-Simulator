#include "stdafx.h"
#include "Thread.h"
#include <process.h>    /* _beginthread, _endthread */


// Thread cleanup function - waits for the main thread to terminate then delete the thread object.
static unsigned __stdcall cleanup(void* obj) {
	Thread* pThread = static_cast<Thread*>(obj);
	pThread->waitToFinish();
	delete pThread;
	return 0;
}

Thread::Thread(bool autostart)
	: threadHandle(0)
	, threadId(0)
	, shouldTerminate(false)
{
	if (autostart) {
		start();
	}
}


Thread::~Thread()
{
	::CloseHandle(threadHandle);
}

void Thread::start()
{
	threadHandle = ::CreateThread(NULL, 0, &threadFunction, this, 0, &threadId);
}

// Wait for this object to finish.
void Thread::waitToFinish()
{
	::WaitForSingleObject(threadHandle, INFINITE);
}

void Thread::stop()
{
	shouldTerminate = true;
}

// Main thread function, calls the run() method.
DWORD WINAPI Thread::threadFunction(void * obj) {
	Thread* pThread = static_cast<Thread*>(obj);
	unsigned ret =  pThread->run();
	return ret;
}

