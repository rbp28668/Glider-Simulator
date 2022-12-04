#include "stdafx.h"
#include "PanelScripting.h"
#include "PanelMessageHandler.h"

void PanelScripting::registerMethods(Lua& lua)
{
	Lua::Module module = lua.startModule("panel");
	module.add("send", PanelScripting::send);
}

int PanelScripting::send(lua_State* L)
{
	std::string host(luaL_checkstring(L, 1));
	int port = (int) (luaL_checkinteger(L, 2));
	std::string cmd(luaL_checkstring(L, 3));
	std::string err;
	PanelMessageHandler::send(host, port, cmd, err);
	return 0;
}
