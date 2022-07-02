#pragma once
#include "Lua.h"

class LogScripting
{
public:
	static void registerMethods(Lua& lua);

private:
	static int  info(lua_State* L);
	static int  warn(lua_State* L);
	static int  error(lua_State* L);

};

