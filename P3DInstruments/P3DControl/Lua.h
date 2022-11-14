#pragma once

#include <string>
#include <set>
#include <map>

#include "Simulator.h"
#include "BlockingQueue.h"
#include "Thread.h"

extern "C" {
#include "../lua-5.4.4/src/lua.h"
#include "../lua-5.4.4/src/lauxlib.h"
#include "../lua-5.4.4/src/lualib.h"
}

class Simulator;

class Lua
{
	// Implementation class to handle system events from P3D.
	class LuaEvents : public Prepar3D::SystemEventHandler {
		Lua* lua;
		std::map<long, std::string>registeredEvents;

	public:
		virtual void handleEvent(SIMCONNECT_RECV_EVENT* evt);
		virtual void quitEvent();

		LuaEvents(Lua* lua) : lua(lua) {}
		~LuaEvents();

		long addEvent(const std::string& name);
	};

	class ScheduleThread : public Thread {
		Lua* lua;
		bool stop;
		CriticalSection cs;
		std::map<long, std::string>timedEvents;
		DWORD startTime;
	public:
		ScheduleThread(Lua* lua);
		~ScheduleThread();
		void schedule(long mS, const std::string& eventName);
		void scheduleFromNow(long mS, const std::string& eventName);
		virtual unsigned run();
		
	};

	// Simulator this implementation is tied to.
	Simulator* pSim;

	// Core Lua state.
	lua_State* L;
	
	// Provide a queue between the p3D event source (or timer) and lua code
	BlockingQueue<std::string> eventQueue;

	// Set as a P3D event handler to receive system events and push onto the queue.
	LuaEvents* eventHandler;

	// Scheduler thread for timed events
	ScheduleThread scheduler;


public:
	
	/// <summary>
	/// Helper class to create Lua modules with C functions and constant enums.
	/// </summary>
	class Module {
		Lua* lua;
		const char* name;
	
	public:
		Module(Lua* lua, const char* name);
		~Module();
		void add(const char* name, lua_CFunction fn);
		void add(const char* name, int value);
	};

	/// <summary>
	/// Background thread to tidy up instances of LuaThread although
	/// will work for any subclass of Thread.
	/// </summary>
	class BackgroundThread : public Thread{

		CriticalSection threadsGuard;
		std::list<Thread*> threads;
		void cleanup();
	public:
		BackgroundThread();
		~BackgroundThread();
		void registerForCleanup(Thread* instance);
		virtual unsigned int run();
	};

	static BackgroundThread cleanup;

	Simulator* getSim() { return pSim; }

	Lua(Simulator* pSim);
	~Lua();

	Module startModule(const char* name);

	// Retrieve this instance from the lua registry.
	static Lua* get(lua_State* L);

	void registerMethod(const char* name, lua_CFunction);
	int run(char** lines, std::string& errorMessage);
	int runFile(const std::string& fileName, std::string& errorMessage);

	int loadFile(const std::string& fileName, std::string& errorMessage);
	int run(std::string& errorMessage);

	// methods for lua functions to tie into.
	void subscribeToEvent(const std::string& eventName);
	std::string nextEvent() { return eventQueue.dequeue(); }
	bool hasEvent() { return !eventQueue.empty(); }
	void scheduleEvent(long mS, const std::string& eventName);
	void scheduleFromNow(long mS, const std::string& eventName);

	// Raise an event via the event queue.
	void raiseEvent(const std::string& eventName) { eventQueue.enqueue(eventName); }
};

