#pragma once
#include "Lua.h"

class IgcScripting
{
public:
	static void registerMethods(Lua& lua);

private:
	static int  list(lua_State* L);
	static int  traffic(lua_State* L);
	static int  replay(lua_State* L);
	static int  clear(lua_State* L);
	static int  start(lua_State* L);
	static int  stop(lua_State* L);

};

