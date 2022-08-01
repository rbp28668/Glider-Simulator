#include "stdafx.h"
#include "Lua.h"

#include "EventMessageScripting.h"
#include "FailureScripting.h"
#include "IgcScripting.h"
#include "LogScripting.h"
#include "PanelScripting.h"
#include "PositionScripting.h"
#include "RecordScripting.h"
#include "ScenarioScripting.h"
#include "ThermalScripting.h"
#include "TrafficScripting.h"
#include "WeatherScripting.h"
#include "SimScripting.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <assert.h>
#include <timeapi.h>

#pragma comment(lib, "Winmm.lib") // need multimedia library for timer functions

// Single instance of cleanup background thread.
Lua::BackgroundThread Lua::cleanup;


/////////////////////////////////////////////////////////////////////////
// Module
/////////////////////////////////////////////////////////////////////////


// Start the table that will hold the values for a module
Lua::Module::Module(Lua* lua, const char* name)
    : lua(lua)
    , name(name)
{
    lua_newtable(lua->L);
}

// Finish the table and register as global
Lua::Module::~Module()
{
    lua_setglobal(lua->L, name);
}

// Add a function to the current module table.
void Lua::Module::add(const char* name, lua_CFunction fn)
{
    lua_pushstring(lua->L, name);
    lua_pushcfunction(lua->L, fn);
    lua_settable(lua->L, -3);
}

// Add an integer to the current module table.
// Intended for constants / enums (which will be mutable)
void Lua::Module::add(const char* name, int value)
{
    lua_pushstring(lua->L, name);
    lua_pushinteger(lua->L, value);
    lua_settable(lua->L, -3);
}

/////////////////////////////////////////////////////////////////////////
// Core Lua
/////////////////////////////////////////////////////////////////////////


Lua::Lua(Simulator* pSim)
    : L(0)
    , pSim(pSim)
    , eventHandler(0)
    , scheduler(this)
{
    L = luaL_newstate();
    *(Simulator**)lua_getextraspace(L) = pSim;

    // Save a pointer to this in the lua registry.
    // Use with Lua::get() to get pointer back.
    lua_pushlightuserdata(L, (void*)this);
    lua_setfield(L, LUA_REGISTRYINDEX, "P3DLUAKEY");

    luaL_openlibs(L); // standard libraries

    EventMessageScripting::registerMethods(*this);
    FailureScripting::registerMethods(*this);
    IgcScripting::registerMethods(*this);
    LogScripting::registerMethods(*this);
    PanelScripting::registerMethods(*this);
    PositionScripting::registerMethods(*this);
    RecordScripting::registerMethods(*this);
    ScenarioScripting::registerMethods(*this);
    ThermalScripting::registerMethods(*this);
    TrafficScripting::registerMethods(*this);
    WeatherScripting::registerMethods(*this);
    SimScripting::registerMethods(*this);
}

Lua::~Lua()
{
    if (eventHandler) {
        delete eventHandler;
    }

    if (L) {
       lua_close(L);
    }
}

// Starts the definition of a module with the given name
Lua::Module Lua::startModule(const char* name)
{
    return Module(this, name);
}

// Gets a pointer to this Lua object from the Lua registry.
Lua* Lua::get(lua_State* L) {
    int type = lua_getfield(L, LUA_REGISTRYINDEX, "P3DLUAKEY");
    assert(type == LUA_TLIGHTUSERDATA);
    Lua* lua = (Lua*) lua_touserdata(L, -1);
    lua_pop(L, 1);
    return lua;
}

// Sets up a C method as a global with the given name.
void Lua::registerMethod(const char* name, lua_CFunction fn)
{
    lua_pushcfunction(L, fn);
    lua_setglobal(L, name);
}

// Runs the given set of lines as a chunk
int Lua::run(char** lines, std::string& errorMessage)
{
    while (*lines) {
        int error = luaL_loadstring(L, *lines) || lua_pcall(L, 0, 0, 0);
        if (error) {
            size_t len;
            const char* errmsg = lua_tolstring(L, -1, &len);
            errorMessage = std::string(errmsg, len);
            std::cout << errorMessage << std::endl;
            lua_pop(L, 1); // remove error message
            return 1;
        }

        ++lines;
    }
    return 0;
}

// Helper function for retrieving an error message
static void handleError(lua_State* L, std::string& errorMessage) {
    size_t len;
    const char* errmsg = lua_tolstring(L, -1, &len);
    errorMessage = std::string(errmsg, len);
    std::cout << errorMessage << std::endl;
    lua_pop(L, 1); // remove error message
}

// Loads and runs a file.
int Lua::runFile(const std::string& fileName, std::string& errorMessage)
{
    int error = luaL_loadfile(L, fileName.c_str()) || lua_pcall(L, 0, 0, 0);
    if (error) {
        handleError(L, errorMessage);
        return 1;
    }
    return 0;
}

// Loads a file
int Lua::loadFile(const std::string& fileName, std::string& errorMessage)
{
    int error = luaL_loadfile(L, fileName.c_str());
    if (error) {
        handleError(L, errorMessage);
        return 1;
    }
    return 0;
}

// Runs a previously loaded chunk.
int Lua::run(std::string& errorMessage)
{
    int error = lua_pcall(L, 0, 0, 0);
    if (error) {
        handleError(L, errorMessage);
        return 1;
    }
    return 0;
}

void Lua::subscribeToEvent(const std::string& eventName)
{
    if (eventHandler == 0) {
        eventHandler = new LuaEvents(this);
        pSim->registerSystemEventHandler(eventHandler);
    }
    eventHandler->addEvent(eventName);
}

void Lua::scheduleEvent(long mS, const std::string& eventName)
{
    scheduler.schedule(mS, eventName);
}

/////////////////////////////////////////////////////////////////////////
// LuaEvents
// Provides an interface into P3D events to allow them to be raised
// as string events.
/////////////////////////////////////////////////////////////////////////

void Lua::LuaEvents::handleEvent(SIMCONNECT_RECV_EVENT* evt)
{
    long eventId = evt->uEventID;
    auto iter = registeredEvents.find(eventId);
    if (iter != registeredEvents.end()) {
        lua->eventQueue.enqueue(iter->second);
    }
}

void Lua::LuaEvents::quitEvent()
{
    lua->raiseEvent("Quit");
}

Lua::LuaEvents::~LuaEvents()
{   
    for (const auto& kv : registeredEvents) {
        long eventId = kv.first;
        lua->pSim->unsubscribeFromSystemEvent(eventId);
    }
    lua->pSim->unRegisterSystemEventHandler(this);
}

long Lua::LuaEvents::addEvent(const std::string& name)
{
    long id = lua->pSim->subscribeToSystemEvent(name.c_str());
    registeredEvents[id] = name;
    return id;
}

/////////////////////////////////////////////////////////////////////////
// Lua::ScheduleThread
// Provides a single scheduling thread per lua instance.  This allows
// lua programs to schedule events for given points in time at which
// point an event message (string) gets raised with the core lua
// instance.  It's up to the lua program to wait for events and handle
// appropriately. 
// The lifetime of this object should be contained within the Lua object
// so that no special cleanup is necessary.
/////////////////////////////////////////////////////////////////////////

/// <summary>
/// Creates a scheduler thread for a given Lua object.
/// </summary>
/// <param name="lua"></param>
Lua::ScheduleThread::ScheduleThread(Lua* lua)
    : lua(lua)
    , stop(false)
    , Thread(true) // start automatically
{
    startTime = ::timeGetTime();
}

/// <summary>
/// Triggers shutdown of the scheduling thread and waits for it to terminate.
/// </summary>
Lua::ScheduleThread::~ScheduleThread()
{
    stop = true;
    this->waitToFinish();
    if (lua->getSim()->isVerbose()) {
        std::cout << "Lua scheduler thread deleted" << std::endl;
    }

}

/// <summary>
/// Schedules an event to be raised at a given time in mS after this scheduler 
/// was created.
/// </summary>
/// <param name="mS"> is the number of mS after the start at which the event should be raised.</param>
/// <param name="eventName">is the event string that should be raised.</param>
void Lua::ScheduleThread::schedule(long mS, const std::string& eventName)
{
    CriticalSection::Lock lock(cs);
    timedEvents.insert(std::make_pair(mS, eventName));
}

/// <summary>
/// Main processing function for the scheduler thread.  This loops checking
/// to see if any events should be raised until the stop flag is set. Note 
/// that deleting the ScheduleThread will set the stop flag and the destructor
/// will wait for this to terminate.
/// </summary>
/// <returns></returns>
unsigned Lua::ScheduleThread::run()
{
    while (running()) {
        long mS = ::timeGetTime() - startTime;
        bool found = false;
        std::string event;
        {
            CriticalSection::Lock lock(cs);
            auto iter = timedEvents.begin(); // lowest key if any
            if (iter != timedEvents.end() && iter->first < mS) {
                event = iter->second;
                timedEvents.erase(iter);
                found = true;
            }
        }
        if (found) {
            lua->raiseEvent(event);
        }
        else {
            ::Sleep(100);
        }
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////
// Lua::BackgroundThread
// Single instance for cleaning up the Lua threads.
/////////////////////////////////////////////////////////////////////////

/// <summary>
/// 
/// </summary>
Lua::BackgroundThread::BackgroundThread()
    : Thread(false)
{
}

/// <summary>
/// 
/// </summary>
Lua::BackgroundThread::~BackgroundThread()
{
}

/// <summary>
/// 
/// </summary>
/// <param name="instance"></param>
void Lua::BackgroundThread::registerForCleanup(Thread* instance)
{
    CriticalSection::Lock lock(threadsGuard);
    threads.push_back(instance);
}

/// <summary>
/// 
/// </summary>
/// <returns></returns>
unsigned int Lua::BackgroundThread::run()
{
    while (running()) {

        std::vector<Thread*> copy;
        size_t len;
        {
            CriticalSection::Lock lock(threadsGuard);
            len = threads.size();
            copy.resize(len);
            int idx = 0;
            for (auto it = threads.begin(); it != threads.end(); ++it,++idx) {
                copy[idx] = *it;
            }
        }

        HANDLE* handles = new HANDLE [len];
        for (int i = 0; i < len; ++i) {
            handles[i] = copy[i]->handle();
        }

        // https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitformultipleobjects
        DWORD which = ::WaitForMultipleObjects((DWORD)len, handles, FALSE, 100);
        if (which >= WAIT_OBJECT_0 && which < WAIT_OBJECT_0 + len) {
            int idx = which - WAIT_OBJECT_0;
            delete copy[idx];

            {
                CriticalSection::Lock lock(threadsGuard);
                threads.remove(copy[idx]);
            }

        }

        delete[] handles;

    }

    cleanup(); // of anything remaining.

    std::cout << "Cleanup thread exiting" << std::endl;

    return 0;
}

void Lua::BackgroundThread::cleanup() {
    CriticalSection::Lock lock(threadsGuard);

    // Tell threads to stop (gracefully)
    for (auto it = threads.begin(); it != threads.end(); ++it) {
        Thread* pThread = *it;
        pThread->stop();
    }

    // Get all the thread handles and wait up to 1sec for them to all stop
    size_t len = threads.size();
    HANDLE* handles = new HANDLE[len];
    int idx = 0;
    for (auto it = threads.begin(); it != threads.end(); ++it, ++idx) {
        assert(idx < len);
        handles[idx] = (*it)->handle();
    }
    ::WaitForMultipleObjects((DWORD)len, handles, TRUE, 1000);
    delete[] handles;

    // delete thread objects.
    for (auto it = threads.begin(); it != threads.end(); ++it, ++idx) {
        delete(*it);
    }

    
}


