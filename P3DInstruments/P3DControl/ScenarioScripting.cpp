#include "stdafx.h"
#include <sstream>
#include "ScenarioScripting.h"
#include "ScenarioMessageHandler.h"

void ScenarioScripting::registerMethods(Lua& lua)
{
	Lua::Module module = lua.startModule("scenario");
	module.add("load", ScenarioScripting::load);
	module.add("save", ScenarioScripting::save);
	module.add("list", ScenarioScripting::list);
}

int ScenarioScripting::load(lua_State* L)
{
	Simulator* pSim = *(Simulator**)lua_getextraspace(L);
	std::string filename(luaL_checkstring(L, 1));
    if (!ScenarioMessageHandler::load(pSim, filename)) {
        lua_pushstring(L, "Unable to load scenario");
        lua_error(L);
    }
	return 0;
}

int ScenarioScripting::save(lua_State* L)
{
	Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    std::string file(luaL_checkstring(L,1));
    std::string title(luaL_checkstring(L, 2));
    std::string desc(luaL_checkstring(L, 3));
    if (!ScenarioMessageHandler::save(pSim, file, title, desc)) {
        lua_pushstring(L,"Unable to save scenario");
        lua_error(L);
    }
	return 0;
}

int ScenarioScripting::list(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);

    try {
        std::string filter(luaL_checkstring(L, 1));
        File::ListT files;
        ScenarioMessageHandler::list(pSim, filter, files);

        lua_newtable(L);
        int idx = 1;
        for (File::ListT::iterator it = files.begin(); it != files.end(); ++it) {

            ScenarioMessageHandler::FileInfo fileInfo;
            ScenarioMessageHandler::readScenario(pSim, *it, fileInfo);

            lua_newtable(L);
            lua_pushstring(L, "filename");
            lua_pushstring(L, fileInfo.filename.c_str());
            lua_settable(L, -3);

            lua_pushstring(L, "title");
            lua_pushstring(L, fileInfo.title.c_str());
            lua_settable(L, -3);

            lua_pushstring(L, "description");
            lua_pushstring(L, fileInfo.description.c_str());
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
