#pragma once
#include "Lua.h"
class PositionScripting
{
public:
	static void registerMethods(Lua& lua);
private:

	static int current(lua_State* L);
	static int available(lua_State* L);
	static int start(lua_State* L);
	static int stop(lua_State* L);
	static int clear(lua_State* L);
	static int set(lua_State* L);
	static int back(lua_State* L);
	static int load(lua_State* L);
	static int save(lua_State* L);
	static int list(lua_State* L);
	static int up(lua_State* L);
	static int down(lua_State* L);
};

