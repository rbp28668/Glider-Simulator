#pragma once
#include "CommandInterpreter.h"
#include "MessageHandler.h"
class ScenarioMessageHandler :
	public MessageHandler
{

public:
	ScenarioMessageHandler(Prepar3D* p3d);
	virtual ~ScenarioMessageHandler();

	virtual void run(const std::string& cmd, const APIParameters& params, std::string& output);

	void loadScenario(const std::string& file, std::string& output);
	void saveScenario(const std::string& file, const std::string& title, const std::string& description, std::string& output);
	void listScenarios(const std::string& filter, std::string& output);

};

