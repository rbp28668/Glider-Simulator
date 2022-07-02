#pragma once
#include "Lua.h"

class PanelScripting
{
public:
	static void registerMethods(Lua& lua);

private:
	static int  send(lua_State* L);

};

