#pragma once
#include "CommandInterpreter.h"
#include "MessageHandler.h"
#include "Folder.h"
#include <list>

class File;
class JSONWriter;
class Simulator;

class ScenarioMessageHandler :
	public MessageHandler
{
public:
	struct FileInfo {
		std::string filename;
		std::string title;
		std::string description;
	};


	ScenarioMessageHandler(Prepar3D* p3d);
	virtual ~ScenarioMessageHandler();

	static void readScenario(Simulator* pSim, File& file, FileInfo& fileInfo);
	static bool load(Simulator* pSim, const std::string& file);
	static bool save(Simulator* pSim, const std::string& file, const std::string& title, const std::string& description);
	static void list(const std::string& filter, File::ListT& files);

	virtual void run(const std::string& cmd, const APIParameters& params, std::string& output);
private:
	void loadScenario(const std::string& file, std::string& output);
	void saveScenario(const std::string& file, const std::string& title, const std::string& description, std::string& output);
	void listScenarios(const std::string& filter, std::string& output);

};

