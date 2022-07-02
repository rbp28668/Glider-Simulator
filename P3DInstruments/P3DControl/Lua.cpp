#include "stdafx.h"
#include "Lua.h"

#include "EventMessageScripting.h"
#include "FailureScripting.h"
#include "IgcScripting.h"
#include "LogScripting.h"
#include "PanelScripting.h"
#include "PositionScripting.h"
#include "RecordScripting.h"
#include "ScenarioScripting.h"
#include "ThermalScripting.h"
#include "TrafficScripting.h"
#include "WeatherScripting.h"

#include <iostream>
#include <fstream>
#include <sstream>


// Start the table that will hold the values for a module
Lua::Module::Module(Lua* lua, const char* name)
    : lua(lua)
    , name(name)
{
    lua_newtable(lua->L);
}

// Finish the table and register as global
Lua::Module::~Module()
{
    lua_setglobal(lua->L, name);
}

// Add a function to the current module table.
void Lua::Module::add(const char* name, lua_CFunction fn)
{
    lua_pushstring(lua->L, name);
    lua_pushcfunction(lua->L, fn);
    lua_settable(lua->L, -3);
}

// Add an integer to the current module table.
// Intended for constants / enums (which will be mutable)
void Lua::Module::add(const char* name, int value)
{
    lua_pushstring(lua->L, name);
    lua_pushinteger(lua->L, value);
    lua_settable(lua->L, -3);
}


Lua::Lua(Simulator* pSim) : L(0)
{
    L = luaL_newstate();
    *(Simulator**)lua_getextraspace(L) = pSim;

    luaL_openlibs(L); // standard libraries

    EventMessageScripting::registerMethods(*this);
    FailureScripting::registerMethods(*this);
    IgcScripting::registerMethods(*this);
    LogScripting::registerMethods(*this);
    PanelScripting::registerMethods(*this);
    PositionScripting::registerMethods(*this);
    RecordScripting::registerMethods(*this);
    ScenarioScripting::registerMethods(*this);
    ThermalScripting::registerMethods(*this);
    TrafficScripting::registerMethods(*this);
    WeatherScripting::registerMethods(*this);
}

Lua::~Lua()
{
    if (L) {
       lua_close(L);
    }
}

Lua::Module Lua::startModule(const char* name)
{
    return Module(this, name);
}

void Lua::registerMethod(const char* name, lua_CFunction fn)
{
    lua_pushcfunction(L, fn);
    lua_setglobal(L, name);
}

int Lua::run(char** lines, std::string& errorMessage)
{
    while (*lines) {
        int error = luaL_loadstring(L, *lines) || lua_pcall(L, 0, 0, 0);
        if (error) {
            size_t len;
            const char* errmsg = lua_tolstring(L, -1, &len);
            errorMessage = std::string(errmsg, len);
            std::cout << errorMessage << std::endl;
            lua_pop(L, 1); // remove error message
            return 1;
        }

        ++lines;
    }
    return 0;
}

int Lua::runFile(const std::string& fileName, std::string& errorMessage)
{
         int error = luaL_loadfile(L, fileName.c_str()) || lua_pcall(L, 0, 0, 0);
        if (error) {
            size_t len;
            const char* errmsg = lua_tolstring(L, -1, &len);
            errorMessage = std::string(errmsg, len);
            std::cout << errorMessage << std::endl;
            lua_pop(L, 1); // remove error message
            return 1;
        }
    return 0;
}

