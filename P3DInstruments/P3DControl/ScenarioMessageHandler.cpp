#include "stdafx.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include "..//rapidxml-1.13/rapidxml.hpp"
#include "..//P3DCommon/Prepar3D.h"
#include "ScenarioMessageHandler.h"
#include "APIParameters.h"
#include "Folder.h"
#include "JSONWriter.h"

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

void ScenarioMessageHandler::readScenario(File& file, JSONWriter& json) {

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
		if (p3d->isVerbose()) {
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

	json.object();
	json.add("filename", name);
	json.add("title", title);
	json.add("description", description);
	json.end(); // of the scenario object

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
		Directory p3dFolder = documents.sub(Prepar3D::DOCUMENTS);
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
		readScenario(*it, json);
	}


}

