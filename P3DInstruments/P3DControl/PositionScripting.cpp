#include "stdafx.h"
#include <sstream>
#include "PositionScripting.h"
#include "PositionMessageHandler.h"
#include "Folder.h"

void PositionScripting::registerMethods(Lua& lua)
{
    Lua::Module module = lua.startModule("position");
    module.add("current", PositionScripting::current);
    module.add("available", PositionScripting::available);
    module.add("start", PositionScripting::start);
    module.add("stop", PositionScripting::stop);
    module.add("clear", PositionScripting::clear);
    module.add("set", PositionScripting::set);
    module.add("back", PositionScripting::back);
    module.add("load", PositionScripting::load);
    module.add("save", PositionScripting::save);
    module.add("list", PositionScripting::list);
    module.add("up", PositionScripting::up);
    module.add("down", PositionScripting::down);

}

int PositionScripting::current(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    SimState::Data data = PositionMessageHandler::show(pSim);
    lua_newtable(L);

    lua_pushstring(L, "airspeed");
    lua_pushnumber(L, data.Airspeed);
    lua_settable(L, -3);

    lua_pushstring(L, "altitude");
    lua_pushnumber(L, data.Altitude);
    lua_settable(L, -3);

    lua_pushstring(L, "bank");
    lua_pushnumber(L, data.Bank);
    lua_settable(L, -3);

    lua_pushstring(L, "heading");
    lua_pushnumber(L, data.Heading);
    lua_settable(L, -3);

    lua_pushstring(L, "latitude");
    lua_pushnumber(L, data.Latitude);
    lua_settable(L, -3);

    lua_pushstring(L, "longitude");
    lua_pushnumber(L, data.Longitude);
    lua_settable(L, -3);

    lua_pushstring(L, "onGround");
    lua_pushnumber(L, data.OnGround);
    lua_settable(L, -3);

    lua_pushstring(L, "pitch");
    lua_pushnumber(L, data.Pitch);
    lua_settable(L, -3);

    return 1;
}

int PositionScripting::available(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    int len = PositionMessageHandler::available(pSim);
    lua_pushinteger(L, len);
    return 1;
}

int PositionScripting::start(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    int len = PositionMessageHandler::start(pSim);
    lua_pushinteger(L, len);
    return 1;
}

int PositionScripting::stop(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    PositionMessageHandler::stop(pSim);
    return 0;
}

int PositionScripting::clear(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    PositionMessageHandler::clearHistory(pSim);
    return 0;
}

int PositionScripting::set(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    int count = (int) luaL_checkinteger(L, 1);
    PositionMessageHandler::set(pSim, count);
    return 0;
}

int PositionScripting::back(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    int count = (int) luaL_checkinteger(L, 1);
    PositionMessageHandler::back(pSim, count);
    return 0;
}

int PositionScripting::load(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    std::string file(luaL_checkstring(L, 1));
    std::string err;
    if (!PositionMessageHandler::load(pSim, file, err)) {
        lua_pushstring(L, err.c_str());
        lua_error(L);
    }
    return 0;
}

int PositionScripting::save(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    std::string file(luaL_checkstring(L, 1));
    std::string title(luaL_checkstring(L, 2));
    std::string description(luaL_checkstring(L, 3));
    PositionMessageHandler::save(pSim, file, title, description);
    return 0;
}

int PositionScripting::list(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);

    try {
        std::string filter(luaL_checkstring(L, 1));
        File::ListT files;
        PositionMessageHandler::list(pSim, filter, files);

        lua_newtable(L);
        int idx = 1;
        for (File::ListT::iterator it = files.begin(); it != files.end(); ++it) {

            std::string title;
            std::string description;

            PositionMessageHandler::getPositionMetadata(pSim, *it, title, description);

            lua_newtable(L);
            lua_pushstring(L, "filename");
            lua_pushstring(L, it->name().c_str());
            lua_settable(L, -3);

            lua_pushstring(L, "title");
            lua_pushstring(L, title.c_str());
            lua_settable(L, -3);

            lua_pushstring(L, "description");
            lua_pushstring(L, description.c_str());
            lua_settable(L, -3);

            lua_seti(L, -2, idx++);
        }
    }
    catch (FileException& fx) {
        std::ostringstream os;
        os << fx.what() << " (" << fx.err() << ")";
        lua_pushstring(L, os.str().c_str());
        lua_error(L);
        return 0;
    }

    return 1;
}

int PositionScripting::up(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    int feet = (int) luaL_checkinteger(L, 1);
    PositionMessageHandler::up(pSim, feet);
    return 0;
}

int PositionScripting::down(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    int feet = (int) luaL_checkinteger(L, 1);
    PositionMessageHandler::down(pSim, feet);
    return 0;
}
