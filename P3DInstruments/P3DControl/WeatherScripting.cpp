#include "stdafx.h"
#include <sstream>
#include "WeatherScripting.h"
#include "WeatherMessageHandler.h"
#include "Simulator.h"

void WeatherScripting::registerMethods(Lua& lua)
{
	Lua::Module module = lua.startModule("weather");
	module.add("themes", WeatherScripting::themes);
	module.add("theme", WeatherScripting::theme);
	module.add("global", WeatherScripting::global);
	module.add("custom", WeatherScripting::custom);
	module.add("request", WeatherScripting::request);
	module.add("requestGlobal", WeatherScripting::requestGlobal);
	module.add("refresh", WeatherScripting::refresh);
	module.add("add", WeatherScripting::add);
	module.add("addHere", WeatherScripting::addHere);
	module.add("stations", WeatherScripting::stations);
	module.add("update", WeatherScripting::update);
	module.add("updateGlobal", WeatherScripting::updateGlobal);
	module.add("set", WeatherScripting::set);
	module.add("setGlobal", WeatherScripting::setGlobal);
	module.add("asTable", WeatherScripting::asTable);

}

// Tries to set a field from a table identified by tableIndex.
static bool processMetarParam(lua_State* L, int tableIndex, Metar::FieldType fieldType, Metar& metar, bool valid) {
	if (!valid) return false; // abort processing if already failed.

	const char* pszKey = Metar::typeName(fieldType);
	lua_pushstring(L, pszKey);
	int type = lua_gettable(L, 1);
	if (type != LUA_TNIL) {
		std::string value(luaL_checkstring(L, -1));
		valid = metar.setField(fieldType, value);
	}
	lua_pop(L, 1);
	return valid;
}

// Initialises a Metar object from a Lua table.
static void processMetarTable(lua_State* L, int tableIndex, Metar& metar) {
	bool valid = true;
	for (int i = 0; i < Metar::FIELD_COUNT; ++i) {
		processMetarParam(L, tableIndex, Metar::type(i), metar, valid);
	}
}

// List all weather themes
int WeatherScripting::themes(lua_State* L)
{
	Simulator* pSim = *(Simulator**)lua_getextraspace(L);

	try {
		std::string filter(luaL_checkstring(L, 1));
		File::ListT files;
		WeatherMessageHandler::listThemes(files);

		lua_newtable(L);
		int idx = 1;
		for (File::ListT::iterator it = files.begin(); it != files.end(); ++it) {

			std::string name;
			std::string title;
			std::string description;
			WeatherMessageHandler::processTheme(*it, name, title, description);

			lua_newtable(L);
			lua_pushstring(L, "filename");
			lua_pushstring(L, name.c_str());
			lua_settable(L, -3);

			lua_pushstring(L, "title");
			lua_pushstring(L, title.c_str());
			lua_settable(L, -3);

			lua_pushstring(L, "description");
			lua_pushstring(L, description.c_str());
			lua_settable(L, -3);

			lua_seti(L, -2, idx++);
		}
		return 1;
	}
	catch (FileException& fx) {
		std::ostringstream os;
		os << fx.what() << " (" << fx.err() << ")";
		lua_pushstring(L, os.str().c_str());
		lua_error(L);
		return 0;
	}
}

// Load a predefined weather theme by name.
int WeatherScripting::theme(lua_State* L)
{
	Simulator* pSim = *(Simulator**)lua_getextraspace(L);
	std::string themeName(luaL_checkstring(L, 1));
	if (!WeatherMessageHandler::setTheme(pSim, themeName.c_str())) {
		lua_pushstring(L, "Unable to select weather theme");
	}
	return 0;
}

// Select global weather
int WeatherScripting::global(lua_State* L)
{
	Simulator* pSim = *(Simulator**)lua_getextraspace(L);
	if (!WeatherMessageHandler::setGlobal(pSim)) {
		lua_pushstring(L, "Unable to select global weather");
	}
	return 0;
}

// Select custom weather
int WeatherScripting::custom(lua_State* L)
{
	Simulator* pSim = *(Simulator**)lua_getextraspace(L);
	if (!WeatherMessageHandler::setCustom(pSim)) {
		lua_pushstring(L, "Unable to select custom weather");
	}
	return 0;
}

// Get the metar data for a given weather station
// returns metar string and name.
int WeatherScripting::request(lua_State* L)
{
	Simulator* pSim = *(Simulator**)lua_getextraspace(L);
	std::string name;
	std::string icao(luaL_checkstring(L, 1));
	std::string metar = WeatherMessageHandler::requestMetar(pSim, icao, name);
	lua_pushstring(L, metar.c_str());
	lua_pushstring(L, name.c_str());
	return 2;
}

// Gets the metar string for the GLOBAL weather
int WeatherScripting::requestGlobal(lua_State* L)
{
	Simulator* pSim = *(Simulator**)lua_getextraspace(L);
	std::string metar = WeatherMessageHandler::requestGlobalMetar(pSim);
	lua_pushstring(L, metar.c_str());
	return 1;
}

// refresh weather state
int WeatherScripting::refresh(lua_State* L)
{
	Simulator* pSim = *(Simulator**)lua_getextraspace(L);
	if (!WeatherMessageHandler::refresh(pSim)) {
		lua_pushstring(L, "Unable to refresh weather");
	}
	return 0;
}

// Add station to list to report on
int WeatherScripting::add(lua_State* L)
{
	Simulator* pSim = *(Simulator**)lua_getextraspace(L);
	std::string icao(luaL_checkstring(L, 1));
	WeatherMessageHandler::addStation(pSim, icao);
	return 0;
}

// Add a new weather station.
// Optionally provide lat/long, otherwise uses a/c position
int WeatherScripting::addHere(lua_State* L)
{
	Simulator* pSim = *(Simulator**)lua_getextraspace(L);
	int nArgs = lua_gettop(L);
	std::string icao(luaL_checkstring(L, 1));
	std::string name(luaL_checkstring(L, 2));
	if (nArgs == 2) {
		icao = WeatherMessageHandler::addWeatherStationHere(pSim, icao, name);
	}
	else {
		float latitude = (float)luaL_checknumber(L, 3);
		float longitude = (float)luaL_checknumber(L, 4);
		icao = WeatherMessageHandler::addWeatherStationAt(pSim, icao, name, latitude, longitude);
	}

	lua_pushstring(L, icao.c_str());
	return 1;
}

// Gets details of all the tables.
int WeatherScripting::stations(lua_State* L)
{
	Simulator* pSim = *(Simulator**)lua_getextraspace(L);
	
	lua_newtable(L);
	int idx = 1;
	for (auto iter = pSim->weatherStations().begin(); iter != pSim->weatherStations().end(); ++iter) {
		if (iter->first == "GLOB") {
			continue;
		}

		lua_newtable(L);
		lua_pushstring(L, "icao");
		lua_pushstring(L, iter->first.c_str());
		lua_settable(L, -3);

		lua_pushstring(L, "updated");
		lua_pushboolean(L, iter->second->hasUpdate());
		lua_settable(L, -3);

		lua_pushstring(L, "metar");
		lua_pushstring(L, iter->second->lastWeatherReport().text().c_str());
		lua_settable(L, -3);

		lua_seti(L, -2, idx++);
	}
	return 1;
}

// Helper function to get a metar parameter either as a table or a string
static void getMetar(lua_State* L, Metar& m, int index) {
	if (lua_istable(L, index)) {
		processMetarTable(L, 1, m);
	}
	else {
		m.parse(luaL_checkstring(L, index));
	}
}

int WeatherScripting::update(lua_State* L)
{
	Simulator* pSim = *(Simulator**)lua_getextraspace(L);
	Metar m;
	getMetar(L, m, 1);
	DWORD seconds = (DWORD) luaL_checkinteger(L, 2);
	if (!WeatherMessageHandler::update(pSim, m, seconds)) {
		lua_pushstring(L, "Unable to update weather");
		lua_error(L);
	}
	return 0;
}

int WeatherScripting::updateGlobal(lua_State* L)
{
	Simulator* pSim = *(Simulator**)lua_getextraspace(L);
	Metar m;
	getMetar(L, m, 1);
	DWORD seconds = (DWORD) luaL_checkinteger(L, 2);
	if (!WeatherMessageHandler::updateGlobal(pSim, m, seconds)) {
		lua_pushstring(L, "Unable to update global weather");
		lua_error(L);
	}
	return 0;
}



int WeatherScripting::set(lua_State* L)
{
	Simulator* pSim = *(Simulator**)lua_getextraspace(L);
	Metar m;
	getMetar(L, m, 1);
	DWORD seconds = (DWORD) luaL_checkinteger(L, 2);
	if (!WeatherMessageHandler::set(pSim, m, seconds)) {
		lua_pushstring(L, "Unable to set weather");
		lua_error(L);
	}
	return 0;
}

int WeatherScripting::setGlobal(lua_State* L)
{
	Simulator* pSim = *(Simulator**)lua_getextraspace(L);
	Metar m;
	getMetar(L, m, 1);
	DWORD seconds = (DWORD) luaL_checkinteger(L, 2);
	if (!WeatherMessageHandler::setGlobal(pSim, m, seconds)) {
		lua_pushstring(L, "Unable to set global weather");
		lua_error(L);
	}
	return 0;
}

int WeatherScripting::asTable(lua_State* L)
{
	std::string inputMetar(luaL_checkstring(L, 1));
	Metar m(inputMetar);
	lua_newtable(L);
	for (int i = 0; i < Metar::FIELD_COUNT; ++i) {
		Metar::FieldType type = Metar::type(i);
		std::string value = m.getRepeated(type);
		std::string tag = Metar::typeName(type);
		if (!value.empty()) {
			lua_pushstring(L, value.c_str());
			lua_setfield(L, -2, tag.c_str());
		}
	}
	return 1;
}

