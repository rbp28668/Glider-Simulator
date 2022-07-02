#pragma once

#include <string>

extern "C" {
#include "../lua-5.4.4/src/lua.h"
#include "../lua-5.4.4/src/lauxlib.h"
#include "../lua-5.4.4/src/lualib.h"
}

class Simulator;

class Lua
{
	lua_State* L;

public:
	
	class Module {
		Lua* lua;
		const char* name;
	public:
		Module(Lua* lua, const char* name);
		~Module();
		void add(const char* name, lua_CFunction fn);
		void add(const char* name, int value);
	};

	Lua(Simulator* pSim);
	~Lua();

	Module startModule(const char* name);

	void registerMethod(const char* name, lua_CFunction);
	int run(char** lines, std::string& errorMessage);
	int runFile(const std::string& fileName, std::string& errorMessage);
};

