#include "stdafx.h"
#include "IgcScripting.h"
#include "IgcMessageHandler.h"

void IgcScripting::registerMethods(Lua& lua)
{
	Lua::Module module = lua.startModule("igc");
	module.add("list", IgcScripting::list);
	module.add("traffic", IgcScripting::traffic);
	module.add("replay", IgcScripting::replay);
	module.add("clear", IgcScripting::clear);
	module.add("start", IgcScripting::start);
	module.add("stop", IgcScripting::stop);
}

int IgcScripting::list(lua_State* L)
{
	// Takes an optional filter string.
	std::string filter = "";
	if (lua_gettop(L) > 0) {
		size_t len;
		const char* str = luaL_checklstring(L, 1, &len);
		filter = std::string(str, len);
	}

	std::list<std::string> fileList;
	std::string err;
	Simulator* pSim = *(Simulator**)lua_getextraspace(L);
	if (IgcMessageHandler::listFiles(pSim, filter, fileList, err)) {
		lua_newtable(L);
		int idx = 1;
		for (auto it = fileList.begin(); it != fileList.end(); ++it) {
			lua_pushinteger(L, idx++);
			lua_pushstring(L, it->c_str());
			lua_settable(L, -3);
		}
		return 1;
	}
	else {
		lua_pushstring(L, err.c_str());
		lua_error(L);
		return 0;
	}
}

int IgcScripting::traffic(lua_State* L)
{
	Simulator* pSim = *(Simulator**)lua_getextraspace(L);
	std::string file(luaL_checkstring(L, 1));
	std::string type(luaL_checkstring(L, 2));
	bool restart = lua_toboolean(L, 3);
	std::string err;
	if(IgcMessageHandler::launch(pSim, file, type, restart, err)){
		return 0;
	}
	else {
		lua_pushstring(L, err.c_str());
		lua_error(L);
		return 0;
	}
}

int IgcScripting::replay(lua_State* L)
{
	Simulator* pSim = *(Simulator**)lua_getextraspace(L);
	std::string file(luaL_checkstring(L, 1));
	std::string type(luaL_checkstring(L, 2));
	std::string err;
	if (IgcMessageHandler::replay(pSim, file, type, err)) {
		return 0;
	}
	else {
		lua_pushstring(L, err.c_str());
		lua_error(L);
		return 0;
	}
}

int IgcScripting::clear(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    std::string output;
    IgcMessageHandler::clear(pSim, output);
    return 0;
}

int IgcScripting::start(lua_State* L)
{
	Simulator* pSim = *(Simulator**)lua_getextraspace(L);

	size_t len;
	const char* str;

	int interval = (int)luaL_checkinteger(L, 1);

	str = luaL_checklstring(L, 2, &len);
	std::string p1(str, len);

	str = luaL_checklstring(L, 3, &len);
	std::string p2(str, len);

	str = luaL_checklstring(L, 4, &len);
	std::string glider_type(str, len);

	str = luaL_checklstring(L, 5, &len);
	std::string registration(str, len);

	str = luaL_checklstring(L, 6, &len);
	std::string competition_class(str, len);

	str = luaL_checklstring(L, 7, &len);
	std::string competition_id(str, len);

	std::string err;
	if (IgcMessageHandler::start(pSim, interval, p1, p2, glider_type, registration, competition_class, competition_id, err)) {
		return 0;
	}
	else {
		lua_pushstring(L, err.c_str());
		lua_error(L);
		return 0;
	}
}

int IgcScripting::stop(lua_State* L)
{
	Simulator* pSim = *(Simulator**)lua_getextraspace(L);
	std::string err;
	if (IgcMessageHandler::stop(pSim, err)) {
		return 0;
	}
	else {
		lua_pushstring(L, err.c_str());
		lua_error(L);
		return 0;
	}
}
