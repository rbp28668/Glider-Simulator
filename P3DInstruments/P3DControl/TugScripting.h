#pragma once
#include "Lua.h"


class TugScripting
{
public:
	static 	void registerMethods(Lua& lua);
private:
	static int setDesiredSpeed(lua_State* L);
	static int setDesiredHeading(lua_State* L);
	static int waveOff(lua_State* L);
	static int waggle(lua_State* L);
	static int sendCommand(lua_State* L);
	static int hasReleased(lua_State* L);
	static int getAltitude(lua_State* L);
	static int getHeading(lua_State* L);
	static int getLatitude(lua_State* L);
	static int getLongitude(lua_State* L);

	static int getDesiredSpeed(lua_State* L);
	static int getDesiredHeading(lua_State* L);

};

