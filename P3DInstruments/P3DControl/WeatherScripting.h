#pragma once
#include "Lua.h"

class WeatherScripting
{
public:
	static void registerMethods(Lua& lua);
private:
	static int themes(lua_State* L);
	static int theme(lua_State* L);
	static int global(lua_State* L);
	static int custom(lua_State* L);
	static int request(lua_State* L);
	static int requestGlobal(lua_State* L);
	static int refresh(lua_State* L);
	static int add(lua_State* L);
	static int addHere(lua_State* L);
	static int stations(lua_State* L);
	static int update(lua_State* L);
	static int updateGlobal(lua_State* L);
	static int set(lua_State* L);
	static int setGlobal(lua_State* L);
	static int asTable(lua_State* L);
};

