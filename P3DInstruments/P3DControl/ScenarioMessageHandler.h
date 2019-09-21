#pragma once
#include "CommandInterpreter.h"
class ScenarioMessageHandler :
	public MessageHandler
{
	std::string name;
	Prepar3D* p3d;

	void reportSuccess(std::string& output);
	void reportFailure(const char* pszReason, unsigned long code, std::string& output);
public:
	ScenarioMessageHandler(Prepar3D* p3d);
	~ScenarioMessageHandler();
	virtual const std::string& getName();
	virtual void run(const std::string& cmd, const APIParameters& params, std::string& output);

	void loadScenario(const std::string& file, std::string& output);
	void saveScenario(const std::string& file, const std::string& title, const std::string& description, std::string& output);
	void listScenarios(const std::string& filter, std::string& output);

};

