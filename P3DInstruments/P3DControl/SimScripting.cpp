#include "stdafx.h"
#include <timeapi.h>
#include "SimScripting.h"

void SimScripting::registerMethods(Lua& lua)
{
    Lua::Module module = lua.startModule("sim");
    module.add("nextMessage", SimScripting::nextMessage);
    module.add("hasMessage", SimScripting::hasMessage);
    module.add("registerForEvent", SimScripting::registerForEvent);
    module.add("sleep", SimScripting::sleep);
    module.add("now", SimScripting::now);
    module.add("schedule", SimScripting::schedule);
    module.add("scheduleFromNow", SimScripting::scheduleFromNow);

 }

int SimScripting::nextMessage(lua_State* L)
{
    Lua* lua = Lua::get(L);
    std::string msg = lua->nextEvent();
    lua_pushstring(L, msg.c_str());
    return 1;
}

int SimScripting::hasMessage(lua_State* L)
{
    Lua* lua = Lua::get(L);
    bool has = lua->hasEvent();
    lua_pushboolean(L, (has) ? 1 : 0);
    return 1;
}

int SimScripting::registerForEvent(lua_State* L)
{
    std::string eventName(luaL_checkstring(L, 1));
    Lua* lua = Lua::get(L);
    lua->subscribeToEvent(eventName);
    return 0;
}

// sleep(ms) - sleeps for ms milliseconds
int SimScripting::sleep(lua_State* L) {
    DWORD ms = (DWORD)luaL_checkinteger(L, -1);
    ::Sleep(ms);
    return 0;
}

int SimScripting::now(lua_State* L)
{
    lua_pushinteger(L, (lua_Integer) ::timeGetTime());
    return 1;
}

int SimScripting::schedule(lua_State* L)
{
    long mS = (long)luaL_checkinteger(L, -1);
    std::string event(luaL_checkstring(L, -2));
    Lua* lua = Lua::get(L);
    lua->scheduleEvent(mS, event);
    return 0;
}

int SimScripting::scheduleFromNow(lua_State* L)
{
    long mS = (long)luaL_checkinteger(L, -1);
    std::string event(luaL_checkstring(L, -2));
    Lua* lua = Lua::get(L);
    lua->scheduleFromNow(mS, event);
    return 0;
}
