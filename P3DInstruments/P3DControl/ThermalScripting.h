#pragma once
#include "Lua.h"

class ThermalScripting
{
public:
	static void registerMethods(Lua& lua);
private:
	static int drop(lua_State* L);


};

