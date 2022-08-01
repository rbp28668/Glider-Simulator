#pragma once
#include "Lua.h"

class ScenarioScripting
{
public:
	static void registerMethods(Lua& lua);
private:
	static int load(lua_State* L);
	static int save(lua_State* L);
	static int list(lua_State* L);

};

