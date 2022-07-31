#pragma once

// Class to abstract a win32 thread.
class Thread
{
private:
	HANDLE threadHandle;
	DWORD threadId;
	bool shouldTerminate;

	static DWORD WINAPI threadFunction(void * obj);

public:
	Thread(bool autostart = true);
	virtual ~Thread();
	void start();
	virtual unsigned run() = 0;
	void waitToFinish();
	void stop();
	HANDLE handle() { return threadHandle; }
	bool running() { return !shouldTerminate; }

};

