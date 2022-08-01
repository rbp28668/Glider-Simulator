#pragma once
#include "Lua.h"

class EventMessageScripting
{
public:
	static void registerMethods(Lua& lua);

private:
	static int dispatchEvent(lua_State* L);

};

