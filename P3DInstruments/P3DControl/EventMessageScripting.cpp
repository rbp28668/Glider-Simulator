#include "stdafx.h"
#include "EventMessageScripting.h"
#include "EventMessageHandler.h"
#include "Logger.h"

void EventMessageScripting::registerMethods(Lua& lua)
{
	lua.registerMethod("cmd", EventMessageScripting::dispatchEvent);
}

int EventMessageScripting::dispatchEvent(lua_State* L)
{
	Simulator* pSim = *(Simulator**) lua_getextraspace(L);
	
	std::string output;

	size_t len;
	const char* szCmd = luaL_checklstring(L, 1, &len);
	std::string cmd(szCmd, len);

	// make parameter optional
	DWORD dwData = 0;
	if (lua_gettop(L) > 1) {
		dwData = (DWORD) luaL_checkinteger(L, 2);
	}

	if (EventMessageHandler::dispatchEvent(pSim, cmd, dwData, output)) {
		pSim->getLogger()->info(pSim,output);
	}
	else {
		pSim->getLogger()->error(pSim,output);
	}

	return 0; // no results
}
