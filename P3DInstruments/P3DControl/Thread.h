#pragma once

// Class to abstract a win32 thread.
class Thread
{
private:
	uintptr_t threadHandle;
	unsigned thrdaddr;

	static unsigned __stdcall threadFunction(void * obj);
public:
	Thread(bool autostart = true);
	virtual ~Thread();
	void start();
	virtual unsigned run() = 0;
	void waitToFinish();
};

