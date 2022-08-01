#include "stdafx.h"
#include <string>
#include "FailureScripting.h"
#include "FailuresMessageHandler.h"
#include "Simulator.h"

void FailureScripting::registerMethods(Lua& lua)
{
    Lua::Module module = lua.startModule("failures");
    module.add("current", FailureScripting::current);
    module.add("airspeed", FailureScripting::airspeed);
    module.add("altimeter", FailureScripting::altimeter);
    module.add("electrical", FailureScripting::electrical);
    module.add("pitot", FailureScripting::pitot);
    module.add("turn_coordinator", FailureScripting::turnCoordinator);
    module.add("clear_all", FailureScripting::clearAll);
    
    module.add("OK", Failures::OK);
    module.add("BLANK", Failures::BLANK);
    module.add("FAIL", Failures::FAIL);
}

Failures::FAILURE_MODE FailureScripting::get_mode(lua_State* L)
{
    Failures::FAILURE_MODE mode = Failures::FAIL;
    if (lua_isboolean(L, 1)) {
        int val = lua_toboolean(L, 1);
        mode = (val) ? Failures::FAIL : Failures::OK;
    }
    else if (lua_isinteger(L, 1)) {
        switch (lua_tointeger(L, 1)) {
        case 0:
            mode = Failures::OK;
            break;
        case 1:
            mode = Failures::FAIL;
            break;
        case 2:
            mode = Failures::BLANK;
            break;
        }
    }
    return mode;
}

// Adds a named integer entry to a table that is on top of the lua stack.
static void addTableEntry(lua_State* L, const char* field, int value) {
    lua_pushinteger(L, value);
    lua_setfield(L, -2, field);
}

// NOTE - output strings being ignored as always return success anyway.
int FailureScripting::current(lua_State* L) {
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    Failures::Data data = pSim->getFailures()->current();

    lua_createtable(L, 0, 5); // need 5 slots
    addTableEntry(L, "airspeed", data.airspeed);
    addTableEntry(L,"altimeter", data.altimeter);
    addTableEntry(L,"electrical", data.electrical);
    addTableEntry(L,"pitot", data.pitot);
    addTableEntry(L,"turn_coordinator", data.turn_coordinator);
    return 1;  // single table
}


int FailureScripting::airspeed(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    Failures::FAILURE_MODE mode = get_mode(L);
    std::string output;
    FailuresMessageHandler::failAirspeed(pSim, mode, output);
    return 0;
}

int FailureScripting::altimeter(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    Failures::FAILURE_MODE mode = get_mode(L);
    std::string output;
    FailuresMessageHandler::failAltimeter(pSim, mode, output);
    return 0;
}

int FailureScripting::electrical(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    Failures::FAILURE_MODE mode = get_mode(L);
    std::string output;
    FailuresMessageHandler::failElectrical(pSim, mode, output);
    return 0;
}

int FailureScripting::pitot(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    Failures::FAILURE_MODE mode = get_mode(L);
    std::string output;
    FailuresMessageHandler::failPitot(pSim, mode, output);
    return 0;
}

int FailureScripting::turnCoordinator(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    Failures::FAILURE_MODE mode = get_mode(L);
    std::string output;
    FailuresMessageHandler::failTurnCoordinator(pSim, mode, output);
    return 0;
}

int FailureScripting::clearAll(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    std::string output;
    FailuresMessageHandler::clearAll(pSim, output);
    return 0;
}
