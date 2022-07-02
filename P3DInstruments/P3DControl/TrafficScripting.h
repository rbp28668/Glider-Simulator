#pragma once
#include "Lua.h"

class TrafficScripting
{
public:
	static void registerMethods(Lua& lua);
private:
	static int launch(lua_State* L);
	static int aircraft(lua_State* L);

};

