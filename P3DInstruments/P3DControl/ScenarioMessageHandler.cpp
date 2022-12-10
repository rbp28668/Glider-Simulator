#include "stdafx.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include "..//rapidxml-1.13/rapidxml.hpp"
#include "..//P3DCommon/Prepar3D.h"
#include "ScenarioMessageHandler.h"
#include "APIParameters.h"
#include "JSONWriter.h"
#include "Simulator.h"

// Wrapper for dynamically allocateed buffer
// Resource Allocation Is Initialisation pattern.
template<class T> 
class Buffer {
	T* data;
public:
	Buffer(size_t size) { data = new T[size]; }
	~Buffer() { delete[] data; }
	operator T* () { return data; }
};



ScenarioMessageHandler::ScenarioMessageHandler(Prepar3D* p3d)
	: MessageHandler(p3d, "scenario")
{
}

ScenarioMessageHandler::~ScenarioMessageHandler()
{
}

bool ScenarioMessageHandler::load(Simulator* pSim, const std::string& file)
{
	HRESULT hr = ::SimConnect_FlightLoad(pSim->getHandle(), file.c_str());
	return(hr == S_OK);
}

bool ScenarioMessageHandler::save(Simulator* pSim, const std::string& file, const std::string& title, const std::string& description)
{
	HRESULT hr = ::SimConnect_FlightSave(pSim->getHandle(), file.c_str(), title.c_str(), description.c_str(), 0);
	return(hr == S_OK);
}

void ScenarioMessageHandler::list(Simulator* pSim, const std::string& filter, File::ListT& files)
{
	DocumentDirectory documents;
	Directory p3dFolder = documents.sub(pSim->documentsFolder());
	files = p3dFolder.files(files, filter + "*.fxml");

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
	else {
		reportFailure("Scenario command not recognised", 0, output);
	}
}

void ScenarioMessageHandler::loadScenario(const std::string& file, std::string& output)
{
	Simulator* pSim = static_cast<Simulator*>(p3d);
	if (load(pSim, file)) {
		reportSuccess(output);
	} else {
		reportFailure("Unable to load scenario", 0, output);
	}
}

void ScenarioMessageHandler::saveScenario(const std::string& file, const std::string& title, const std::string& description, std::string& output)
{
	Simulator* pSim = static_cast<Simulator*>(p3d);
	if(save(pSim, file.c_str(), title.c_str(), description.c_str())){
		reportSuccess(output);
	} else {
		reportFailure("Unable to save scenario", 0, output);
	}
}

void ScenarioMessageHandler::readScenario(Simulator* pSim, File& file, FileInfo& fileInfo) {

	std::string name = file.name();
	std::string title = name;
	std::string description = "";


	// read the file into memory
	std::ifstream ifs(file);
	ifs.seekg(0, std::ios::end);    
	size_t length = ifs.tellg();          
	ifs.seekg(0, std::ios::beg);    
	Buffer<char> buffer(length + 1);
	ifs.read(buffer, length);       
	buffer[length] = 0;
	
	// Parse it
	rapidxml::xml_document<> doc;    // character type defaults to char

	try {
		doc.parse<0>(buffer);
	}
	catch (rapidxml::parse_error& err) {
		// Errors tend to be benign so just log it.
		std::cout << err.what() << std::endl;
		if (pSim->isVerbose()) {
			char* where = err.where<char>();
			std::cout << where << std::endl;
		}
	}

	// Go down to find the main section.
	rapidxml::xml_node<>* pNode = doc.first_node("SimBase.Document");
	
	if (pNode) {
		pNode = pNode->first_node("Flight.Sections");
	}

	// Look for Section with Name="Main"
	if (pNode) {
		pNode = pNode->first_node("Section");
		while (pNode) {
			rapidxml::xml_attribute<>* pAttr = pNode->first_attribute("Name");
			std::string name = std::string(pAttr->value(), pAttr->value_size());
			if (name == "Main") {
				break;
			}
			pNode = pNode->next_sibling("Section");
		}
	}

	// Look for the properties in the main section
	if (pNode) {
		pNode = pNode->first_node("Property");
		while (pNode) {
			// Get name and value for this property
			rapidxml::xml_attribute<>* pAttr = pNode->first_attribute("Name");
			std::string name = std::string(pAttr->value(), pAttr->value_size());
			pAttr = pNode->first_attribute("Value");
			std::string value = std::string(pAttr->value(), pAttr->value_size());

			if (name == "Title") {
				title = value;
			}
			else if (name == "Description") {
				description = value;
			}

			pNode = pNode->next_sibling("Property");
		}
	}

	fileInfo.filename = name;
	fileInfo.title = title;
	fileInfo.description = description;
}

// Lists any scenarios (in JSON response written to output) that correspond to the filter.
// If filter is empty then all the .fxml files are written, otherwise only the files
// that start with the filter are written.
void ScenarioMessageHandler::listScenarios(const std::string& filter, std::string& output)
{
	File::ListT files;
	Simulator* pSim = static_cast<Simulator*>(p3d);
	try {
		list(pSim, filter, files);
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
		FileInfo fileInfo;
		readScenario(pSim, *it, fileInfo);
		json.object();
		json.add("filename", fileInfo.filename);
		json.add("title", fileInfo.title);
		json.add("description", fileInfo.description);
		json.end(); // of the scenario object
	}


}

