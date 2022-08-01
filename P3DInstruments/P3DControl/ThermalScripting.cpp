#include "stdafx.h"
#include "ThermalScripting.h"
#include "ThermalMessageHandler.h"

void ThermalScripting::registerMethods(Lua& lua)
{
	Lua::Module module = lua.startModule("thermal");
	module.add("drop", ThermalScripting::drop);
}

int ThermalScripting::drop(lua_State* L)
{
	Simulator* pSim = *(Simulator**)lua_getextraspace(L);
	int args = lua_gettop(L);
	bool ok = true;
	float radius = (float)luaL_checknumber(L, 1);
	float height = (float)luaL_checknumber(L, 2);
	float coreRate = (float)luaL_checknumber(L, 3);
	if (args == 3) {
		ok = ThermalMessageHandler::drop(pSim, radius, height, coreRate);
	}
	else {
		float lat = (float)luaL_checknumber(L, 4);
		float lon = (float)luaL_checknumber(L, 5);
		ok = ThermalMessageHandler::drop(pSim, radius, height, coreRate, lat, lon);
	}
		
	if (!ok) {
		lua_pushstring(L, "Unable to create thermal");
		lua_error(L);
	}
	return 0;
}
