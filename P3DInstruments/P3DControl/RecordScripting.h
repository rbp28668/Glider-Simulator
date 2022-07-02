#pragma once
#include "Lua.h"

class RecordScripting
{
public:
	static void registerMethods(Lua& lua);
private:
	static int record(lua_State* L);
	static int play(lua_State* L);
	static int stop(lua_State* L);
	static int analyse(lua_State* L);
	static int list(lua_State* L);


};

