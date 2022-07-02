#include "stdafx.h"
#include "LogScripting.h"
#include "Simulator.h"
#include "Logger.h"

void LogScripting::registerMethods(Lua& lua)
{
    Lua::Module module = lua.startModule("log");
    module.add("info", LogScripting::info);
    module.add("warn", LogScripting::warn);
    module.add("error", LogScripting::error);

}

int LogScripting::info(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    std::string message(luaL_checkstring(L, 1));
    pSim->getLogger()->info(message);
    return 0;
}

int LogScripting::warn(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    std::string message(luaL_checkstring(L, 1));
    pSim->getLogger()->warn(message);
    return 0;
}

int LogScripting::error(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    std::string message(luaL_checkstring(L, 1));
    pSim->getLogger()->error(message);
    return 0;
}
