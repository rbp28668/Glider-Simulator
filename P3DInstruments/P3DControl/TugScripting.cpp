#include "stdafx.h"
#include "TugScripting.h"
#include "Tug.h"

void TugScripting::registerMethods(Lua& lua)
{
    Lua::Module module = lua.startModule("tug");
    module.add("setDesiredSpeed", TugScripting::setDesiredSpeed);
    module.add("setDesiredHeading", TugScripting::setDesiredHeading);
    module.add("turn", TugScripting::turn);
    module.add("waveOff", TugScripting::waveOff);
    module.add("waggle", TugScripting::waggle);
    module.add("sendCommand", TugScripting::sendCommand);
    module.add("hasReleased", TugScripting::hasReleased);
    module.add("getAltitude", TugScripting::getAltitude);
    module.add("getHeading", TugScripting::getHeading);
    module.add("getLatitude", TugScripting::getLatitude);
    module.add("getLongitude", TugScripting::getLongitude);
    module.add("getDesiredSpeed", TugScripting::getDesiredSpeed);
    module.add("getDesiredHeading", TugScripting::getDesiredHeading);

}

/// <summary>
/// Lua C function to set the desired speed for the tug.
/// </summary>
/// <param name="L">is the Lua state. Function reads a single numerical value from Lua stack which is the desired speed.</param>
/// <returns>0 - no lua values returned</returns>
int TugScripting::setDesiredSpeed(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    double speed = luaL_checknumber(L, 1);
    Tug* tug = pSim->tug();
    if (tug) {
        tug->setDesiredSpeed(speed);
    }
    return 0;
}

/// <summary>
/// Lua C function to set the desired heading for the tug.
/// </summary>
/// <param name="L">is the Lua state. Function reads a single numerical value from Lua stack which is the desired heading.</param>
/// <returns>0 - no lua values returned</returns>
int TugScripting::setDesiredHeading(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    double heading = luaL_checknumber(L, 1);
    Tug* tug = pSim->tug();
    if (tug) {
        tug->setDesiredHeading(heading);
    }
    return 0;
}

/// <summary>
/// Lua C function to make the tug turn from its current course.  Similar to setDesiredHeading except the heading is relative
/// to the current heading.
/// </summary>
/// <param name="L">is the Lua state. Function reads a single numerical value from Lua stack which is the number of degrees to turn left or right.
/// Positive turns right, negative left.</param>
/// <returns>0 - no lua values returned</returns>
int TugScripting::turn(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    double degrees = luaL_checknumber(L, 1);
    Tug* tug = pSim->tug();
    if (tug) {
        tug->turn(degrees);
    }
    return 0;
}

/// <summary>
/// Lua C function to make the tug signal a wave-off.
/// </summary>
/// <param name="L">is the Lua state.</param>
/// <returns>0 - no lua values returned</returns>
int TugScripting::waveOff(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    Tug* tug = pSim->tug();
    if (tug) {
        tug->waggleWings();
    }
    return 0;
}

/// <summary>
/// Lua C function to make the tug signal a problem
/// </summary>
/// <param name="L">is the Lua state.</param>
/// <returns>0 - no lua values returned</returns>
int TugScripting::waggle(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    Tug* tug = pSim->tug();
    if (tug) {
        tug->waggleRudder();
    }
    return 0;
}
/// <summary>
/// Lua C function to send an event or command to the tug.
/// </summary>
/// <param name="L">is the Lua state. 
/// Param 1 is the name of the event.
/// Param 2 is the parameter for the event.
/// </param>
/// <returns>0 - nothing returned to Lua.</returns>
int TugScripting::sendCommand(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    std::string event(luaL_checkstring(L, 1));
    DWORD param = (DWORD)luaL_checkinteger(L, 2);
    Tug* tug = pSim->tug();
    if (tug) {
        tug->dispatchEvent(event, param);
    }
    return 0;
}

/// <summary>
/// hasReleased is a Lua C function that determines whether the tug has released.
/// </summary>
/// <param name="L">is the Lua state</param>
/// <returns>1 - a single value on the Lua stack that is true if the tug has released, false if no tug or has not released.</returns>
int TugScripting::hasReleased(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    bool released = false;
    Tug* tug = pSim->tug();
    if (tug) {
        released = tug->hasReleased();
    }
    lua_pushboolean(L, released ? 1 : 0);
    return 1;
}

/// <summary>
/// Gets the tug altitude
/// </summary>
/// <param name="L">Lua context.</param>
/// <returns>1 - single Lua return value of altitude in feet.</returns>
int TugScripting::getAltitude(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    double altitude = 0;
    Tug* tug = pSim->tug();
    if (tug) {
        altitude = tug->getAltitude();
    }
    lua_pushnumber(L, altitude);
    return 1;
}

/// <summary>
/// Gets the tug heading
/// </summary>
/// <param name="L">Lua context.</param>
/// <returns>1 - single Lua return value of tug heading.</returns>
int TugScripting::getHeading(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    double heading = 0;
    Tug* tug = pSim->tug();
    if (tug) {
        heading = tug->getHeading();
    }
    lua_pushnumber(L, heading);
    return 1;
}
/// <summary>
/// Gets the tug latitude
/// </summary>
/// <param name="L">Lua context.</param>
/// <returns>1 - single Lua return value of tug latitude.</returns>
int TugScripting::getLatitude(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    double latitude = 0;
    Tug* tug = pSim->tug();
    if (tug) {
        latitude = tug->getLatitude();
    }
    lua_pushnumber(L, latitude);
    return 1;
}
/// <summary>
/// Gets the tug longitude
/// </summary>
/// <param name="L">Lua context.</param>
/// <returns>1 - single Lua return value of altitude in feet.</returns>
int TugScripting::getLongitude(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    double longitude = 0;
    Tug* tug = pSim->tug();
    if (tug) {
        longitude = tug->getLongitude();
    }
    lua_pushnumber(L, longitude);
    return 1;
}

/// <summary>
/// Gets the desired speed
/// </summary>
/// <param name="L"></param>
/// <returns></returns>
int TugScripting::getDesiredSpeed(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    double speed = 0;
    Tug* tug = pSim->tug();
    if (tug) {
        speed = tug->getDesiredSpeed();
    }
    lua_pushnumber(L, speed);
    return 1;
}

/// <summary>
/// Gets the desired heading
/// </summary>
/// <param name="L"></param>
/// <returns></returns>
int TugScripting::getDesiredHeading(lua_State* L)
{
    Simulator* pSim = *(Simulator**)lua_getextraspace(L);
    double heading = 0;
    Tug* tug = pSim->tug();
    if (tug) {
        heading = tug->getDesiredHeading();
    }
    lua_pushnumber(L, heading);
    return 1;
}
