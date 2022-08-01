#include "stdafx.h"
#include <iostream>
#include "LuaThread.h"

LuaThread::LuaThread(Simulator* pSim)
	: Thread(false)  // no autostart
	, lua(pSim)
	, pSim(pSim)
{
	Lua::cleanup.registerForCleanup(this);
}

LuaThread::~LuaThread()
{
	if (pSim->isVerbose()) {
		std::cout << "Lua thread deleted" << std::endl;
	}
}

/// <summary>
///  runs a Lua file by loading it in this thread then starting the thread to 
/// actually run the code.
/// </summary>
/// <param name="fileName"></param>
/// <param name="errorMessage"></param>
/// <returns></returns>
int LuaThread::runFile(const std::string& fileName, std::string& errorMessage)
{
	if (lua.loadFile(fileName, errorMessage) == 0) {
		start();
	}
	else {
		delete this;  // as start not called so won't autodelete.
	}

	return 0;
}

// Main thread function- run the script.
unsigned LuaThread::run()
{
	std::string errorMessage;
	if (lua.run(errorMessage)) {
		// NOP - should probably log
	}

	if (pSim->isVerbose()) {
		std::cout << "Lua thread complete" << std::endl;
	}

	return 0;
}
