#pragma once
#include "MessageHandler.h"
class ScriptMessageHandler :
    public MessageHandler
{
public:
	ScriptMessageHandler(Prepar3D* p3d);
	virtual ~ScriptMessageHandler();

	virtual void run(const std::string& cmd, const APIParameters& params, std::string& output);

	void listScriptFiles(const std::string& filter, std::string& output);
	void runScript(const std::string& file, std::string& output);

};

