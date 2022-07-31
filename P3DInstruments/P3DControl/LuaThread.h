#pragma once
#include "Thread.h"
#include "Lua.h"

class LuaThread :
    public Thread
{
    Lua lua;
    Simulator* pSim;
public:
    LuaThread(Simulator* pSim);
    virtual ~LuaThread();

    int runFile(const std::string& fileName, std::string& errorMessage);

    virtual unsigned run();

};

