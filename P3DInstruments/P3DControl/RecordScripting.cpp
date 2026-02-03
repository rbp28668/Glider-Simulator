#include "stdafx.h"
#include "RecordScripting.h"
#include "RecordMessageHandler.h"

void RecordScripting::registerMethods(Lua& lua)
{
	Lua::Module module = lua.startModule("record");
	module.add("record", RecordScripting::record);
	module.add("play", RecordScripting::play);
	module.add("stop", RecordScripting::stop);
	module.add("analyse", RecordScripting::analyse);
	module.add("list", RecordScripting::list);
}

int RecordScripting::record(lua_State* L)
{
	Simulator* pSim = *(Simulator**)lua_getextraspace(L);
	if (!RecordMessageHandler::start(pSim)) {
		lua_pushstring(L, "Unable to start recording");
		lua_error(L);
		return 0;
	}
	return 0;
}

int RecordScripting::play(lua_State* L)
{
	Simulator* pSim = *(Simulator**)lua_getextraspace(L);
	std::string name(luaL_checkstring(L, 1));
	if (!RecordMessageHandler::playback(pSim, name)) {
		lua_pushstring(L, "Unable to play recording");
		lua_error(L);
		return 0;
	}
	return 0;
}

int RecordScripting::stop(lua_State* L)
{
	Simulator* pSim = *(Simulator**)lua_getextraspace(L);
	std::string title(luaL_checkstring(L, 1));
	std::string description(luaL_checkstring(L, 2));
	if (!RecordMessageHandler::stop(pSim, title, description)) {
		lua_pushstring(L, "Unable to stop recording");
		lua_error(L);
		return 0;
	}
	return 0;
}

int RecordScripting::analyse(lua_State* L)
{
	Simulator* pSim = *(Simulator**)lua_getextraspace(L);
	if (!RecordMessageHandler::analyse(pSim)) {
		lua_pushstring(L, "Unable to analyse flight");
		lua_error(L);
		return 0;
	}
	return 0;
}

int RecordScripting::list(lua_State* L)
{
	Simulator* pSim = *(Simulator**)lua_getextraspace(L);

	File::ListT files;
	RecordMessageHandler::list(pSim,files);
	lua_newtable(L);

	int idx = 1;
	for (auto it = files.begin(); it!=files.end(); ++it) {
		lua_pushstring(L, it->name().c_str());
		lua_seti(L, -2, idx++);
	}
	return 1;
}
