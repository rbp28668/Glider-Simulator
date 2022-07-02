#pragma once
#include "Lua.h"
#include "Failures.h"

class FailureScripting
{
public:
	static void registerMethods(Lua& lua);

private:
	static Failures::FAILURE_MODE get_mode(lua_State* L);
	static int  current(lua_State* L);
	static int  airspeed(lua_State* L);
	static int  altimeter(lua_State* L);
	static int  electrical(lua_State* L);
	static int  pitot(lua_State* L);
	static int  turnCoordinator(lua_State* L);
	static int  clearAll(lua_State* L);

};

