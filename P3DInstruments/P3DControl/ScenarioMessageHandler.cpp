#include "stdafx.h"
#include <sstream>
#include "..//P3DCommon/Prepar3D.h"
#include "ScenarioMessageHandler.h"
#include "APIParameters.h"
#include "Folder.h"
#include "JSONWriter.h"

void ScenarioMessageHandler::reportSuccess(std::string& output)
{
	JSONWriter json(output);
	json.add("status", "OK");
}

void ScenarioMessageHandler::reportFailure(const char* pszReason, unsigned long code, std::string& output)
{
	JSONWriter json(output);
	json.add("status", "FAILED");
	std::ostringstream os;
	os << pszReason << " (" << code << ")";
	json.add("reason", os.str());
}

ScenarioMessageHandler::ScenarioMessageHandler(Prepar3D* p3d)
	: p3d(p3d)
	, name("scenario")
{
}

ScenarioMessageHandler::~ScenarioMessageHandler()
{
}

const std::string& ScenarioMessageHandler::getName()
{
	return name;
}

void ScenarioMessageHandler::run(const std::string& cmd, const APIParameters& params, std::string& output)
{
	if (cmd == "load") {
		std::string file = params.getString("file");
		if (file.empty()) {
			reportFailure("filename must be given", 0, output);
		} else {
			loadScenario(file, output);
		}
	}
	else if (cmd == "save") {
		std::string file = params.getString("file");
		std::string title = params.getString("title");
		std::string desc = params.getString("description");
		if (file.empty()) {
			reportFailure("filename must be given", 0, output);
		} else {
			saveScenario(file, title, desc, output);
		}
	}
	else if (cmd == "list") {
		std::string filter = params.getString("filter");
		listScenarios(filter, output);
	}
}

void ScenarioMessageHandler::loadScenario(const std::string& file, std::string& output)
{
	HRESULT hr = ::SimConnect_FlightLoad(p3d->getHandle(), file.c_str());
	if (hr == S_OK) {
		reportSuccess(output);
	} else {
		reportFailure("Unable to load scenario", hr, output);
	}
}

void ScenarioMessageHandler::saveScenario(const std::string& file, const std::string& title, const std::string& description, std::string& output)
{
	HRESULT hr = ::SimConnect_FlightSave(p3d->getHandle(), file.c_str(), title.c_str(), description.c_str(), 0);
	if (hr == S_OK) {
		reportSuccess(output);
	} else {
		reportFailure("Unable to save scenario", hr, output);
	}
}

// Lists any scenarios (in JSON response written to output) that correspond to the filter.
// If filter is empty then all the .fxml files are written, otherwise only the files
// that start with the filter are written.
void ScenarioMessageHandler::listScenarios(const std::string& filter, std::string& output)
{
	File::ListT files;

	try {
		// E:\Users\rbp28668\Documents\Prepar3D v4 Files
		DocumentDirectory documents;
		Directory p3dFolder = documents.sub("Prepar3D v4 Files");
		files = p3dFolder.files(files, filter + "*.fxml");
	}
	catch (FileException& fx) {
		JSONWriter json(output);
		json.add("status", "FAILED");
		std::ostringstream os;
		os << fx.what() << " (" << fx.err() << ")";
		json.add("reason", os.str());
		return;
	}
		
	JSONWriter json(output);
	json.add("status", "OK");
	json.array("entries");
	for (File::ListT::iterator it = files.begin(); it != files.end(); ++it) {
		json.object("scenario");
		json.add("filename", it->name());
		json.add("title", it->name());
		json.add("description", "");
		json.end(); // of the scenario object
	}


}

