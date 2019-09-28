#include "stdafx.h"
#include <assert.h>
#include "PositionMessageHandler.h"
#include "JSONWriter.h"
#include "APIParameters.h"
#include "Folder.h"
#include "json/json.h"
#include <iostream>
#include <fstream>
#include <sstream>

void PositionMessageHandler::write(const SimState::Data& data, JSONWriter& json)
{
	json.add("latitude", data.Latitude);
	json.add("longitude", data.Longitude);
	json.add("altitude", data.Altitude);
	json.add("pitch", data.Pitch);
	json.add("bank", data.Bank);
	json.add("heading", data.Heading);
	json.add("on_ground", data.OnGround);
	json.add("airspeed", data.Airspeed);
}

void PositionMessageHandler::getPositionMetadata(File& f, std::string& title, std::string& description)
{
	Json::Value root;
	std::ifstream ifs;
	ifs.open(f);

	Json::CharReaderBuilder builder;
	Json::String errs;
	bool ok = parseFromStream(builder, ifs, &root, &errs);
	ifs.close();
	if (ok) {
		if (p3d->isVerbose()) {
			std::cout << root << std::endl;
		}
		assert(root.isObject());
		title = root["title"].asCString();
		description = root["description"].asCString();
	}

}

PositionMessageHandler::PositionMessageHandler(Prepar3D* p3d)
	: MessageHandler(p3d, "position")
	, pSim(static_cast<Simulator*>(p3d))
{
	assert(p3d);
}

PositionMessageHandler::~PositionMessageHandler()
{
}

void PositionMessageHandler::run(const std::string& cmd, const APIParameters& params, std::string& output)
{
	assert(this);

	if (cmd == "current") {
		show(output);
	}
	else if (cmd == "available") {
		available(output);
	}
	else if (cmd == "back") {
		int count = params.getInt("count", 10); // default to 10s if nothing given.
		back(count, output);
	}
	else if (cmd == "load") {
		std::string file = params.getString("file");
		if (file.empty()) {
			reportFailure("filename must be given", 0, output);
			return;
		}
		load(file, output);
	}
	else if (cmd == "save") {
		std::string file = params.getString("file");
		std::string title = params.getString("title");
		std::string desc = params.getString("description");
		if (file.empty()) {
			reportFailure("filename must be given", 0, output);
			return;
		}
		save(file, title, desc, output);
	}
	else if (cmd == "list") {
		std::string filter = params.getString("filter");
		list(filter, output);
	}
	else if (cmd == "up") {
		int feet = params.getInt("feet", 100);
		up(feet, output);
	}
	else if (cmd == "down") {
		int feet = params.getInt("feet", 100);
		down(feet, output);
	}
	else {
		reportFailure("Unrecognised position command", 0, output);
	}
}

void PositionMessageHandler::show(std::string& output)
{
	SimState::Data data = pSim->getState()->current();
	JSONWriter json(output);
	json.add("status", "OK");
	
	json.object("current");
	write(data, json);
	json.end();
}

void PositionMessageHandler::available(std::string& output)
{
	int len = pSim->getState()->historyLength();
	JSONWriter json(output);
	json.add("status", "OK");
	json.add("length", len);
}

void PositionMessageHandler::back(int count, std::string& output)
{
	SimState::Data data = pSim->getState()->history(count);
	pSim->getState()->update(data);
	reportSuccess(output);
}

void PositionMessageHandler::load(const std::string& file, std::string& output)
{
	DocumentDirectory documents;
	Directory p3dFolder = documents.sub(Prepar3D::DOCUMENTS);
	File src = p3dFolder.file(file);

	Json::Value root;
	std::ifstream ifs;
	ifs.open(src);

	Json::CharReaderBuilder builder;
	Json::String errs;
	if (!parseFromStream(builder, ifs, &root, &errs)) {
		reportFailure(errs.c_str(),0,output);
		ifs.close();
		return;
	}
	ifs.close();
	if (p3d->isVerbose()) {
		std::cout << root << std::endl;
	}
	assert(root.isObject());

	// can ignore these
	// Json::Value title = root["title"];
	// Json::Value desc = root.["description"];

	SimState::Data data;
	data.Latitude = root["latitude"].asDouble();
	data.Longitude = root["longitude"].asDouble();
	data.Altitude = root["altitude"].asDouble();
	data.Pitch = root["pitch"].asDouble();
	data.Bank = root["bank"].asDouble();
	data.Heading = root["heading"].asDouble();
	data.OnGround = root["on_ground"].asInt();
	data.Airspeed = root["airspeed"].asInt();

	pSim->getState()->update(data);

	reportSuccess(output);
}

void PositionMessageHandler::save(const std::string& file, const std::string& title, const std::string& description, std::string& output)
{
	SimState::Data data = pSim->getState()->current();
	std::string contents;
	JSONWriter json(contents);
	json.add("title", title);
	json.add("description", description);
	write(data, json);
	json.end();
		
	DocumentDirectory documents;
	Directory p3dFolder = documents.sub(Prepar3D::DOCUMENTS);
	File dest = p3dFolder.file(file);
	std::ofstream ofs(dest);
	ofs << contents << std::endl;
	ofs.close();

	reportSuccess(output);
}

void PositionMessageHandler::list(const std::string& filter, std::string& output)
{
	File::ListT files;
	
	try {
		// E:\Users\rbp28668\Documents\Prepar3D v4 Files
		DocumentDirectory documents;
		Directory p3dFolder = documents.sub("Prepar3D v4 Files");
		files = p3dFolder.files(files, filter + "*.posn");


		JSONWriter json(output);
		json.add("status", "OK");
		json.array("entries");
		for (File::ListT::iterator it = files.begin(); it != files.end(); ++it) {

			File src = p3dFolder.file(*it); // get full path

			std::string title;
			std::string description;
			getPositionMetadata(src, title, description);

			json.object();
			json.add("filename", it->name());
			json.add("title", title);
			json.add("description", description);
			json.end(); // of the scenario object
		}
	}
	catch (FileException& fx) {
		JSONWriter json(output);
		json.add("status", "FAILED");
		std::ostringstream os;
		os << fx.what() << " (" << fx.err() << ")";
		json.add("reason", os.str());
		return;
	}


}

void PositionMessageHandler::up(int feet, std::string& output)
{
	SimState::Data data = pSim->getState()->current();
	
	data.Altitude += feet;
	if (data.Altitude < 0) data.Altitude = 0;
	pSim->getState()->update(data);
	reportSuccess(output);
}

void PositionMessageHandler::down(int feet, std::string& output)
{
	up(-feet, output);
}
