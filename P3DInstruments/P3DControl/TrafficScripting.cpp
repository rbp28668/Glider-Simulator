#include "stdafx.h"
#include "TrafficScripting.h"
#include "TrafficMessageHandler.h"
#include "Folder.h"

void TrafficScripting::registerMethods(Lua& lua)
{
	Lua::Module module = lua.startModule("traffic");
	module.add("launch", TrafficScripting::launch);
	module.add("aircraft", TrafficScripting::aircraft);
}

int TrafficScripting::launch(lua_State* L)
{
	Simulator* pSim = *(Simulator**)lua_getextraspace(L);
	std::string targetName(luaL_checkstring(L, 1));
	std::string targetTailNumber(luaL_checkstring(L, 2));
	double targetRange = luaL_checknumber(L, 3);
	double targetSpeed = luaL_checknumber(L, 4);
	double relativeBearing = luaL_checknumber(L, 5);
	double relativeTargetHeight = luaL_checknumber(L, 6);
	if (!TrafficMessageHandler::launch(pSim, targetName, targetTailNumber, targetRange, targetSpeed, relativeBearing, relativeTargetHeight)) {
		lua_pushstring(L, "Unable to launch traffic");
		lua_error(L);
	}
	return 0;
}

int TrafficScripting::aircraft(lua_State* L)
{
	std::list<std::string> aircraft;
	lua_newtable(L);
	int idx = 1;

	try {

		TrafficMessageHandler::listAircraft(aircraft);
		for (auto it = aircraft.begin(); it != aircraft.end(); ++it) {
			lua_pushstring(L, it->c_str());
			lua_seti(L, -2, idx++);
		}

	}
	catch (FileException& fx) {
		std::string err("Unable to list available aircraft: ");
		err.append(fx.what());
		lua_pushstring(L, err.c_str());
		lua_error(L);
	}

	return 1;
}
