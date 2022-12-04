#pragma once
#include "Lua.h"

class SimScripting
{
public:
	static void registerMethods(Lua& lua);

private:
	static int nextMessage(lua_State* L);
	static int hasMessage(lua_State* L);
	static int registerForEvent(lua_State* L);
	static int sleep(lua_State* L);
	static int now(lua_State* L);
	static int schedule(lua_State* L);
	static int scheduleFromNow(lua_State* L);
};

