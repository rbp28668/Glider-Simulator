#include "stdafx.h"
#include <sstream>
#include <string>
#include <list>
#include "..//P3DCommon/Prepar3D.h"
#include "..//P3DCommon/ExternalSim.h"
#include "IgcMessageHandler.h"
#include "APIParameters.h"
#include "IGCFile.h"
#include "IGCAircraft.h"
#include "Folder.h"
#include "JSONWriter.h"
#include "Simulator.h"

IgcMessageHandler::IgcMessageHandler(Prepar3D* p3d) 
	: MessageHandler(p3d, "igc")
{

}

IgcMessageHandler::~IgcMessageHandler()
{
}

void IgcMessageHandler::run(const std::string& cmd, const APIParameters& params, std::string& output)
{
	Simulator* pSim = static_cast<Simulator*>(p3d);

	if (cmd == "list") {
		std::string filter = params.getString("filter");
		listAvailableIGCFiles( pSim, filter, output);
	}
	else if (cmd == "traffic") {
		std::string file = params.getString("igc");
		std::string type = params.getString("type");
		bool restart = params.getBool("restart");
		if (file.empty() || type.empty()) {
			reportFailure("Must supply igc and type parameters to launch traffic", 0, output);
		}
		else {
			launchIgcTraffic(file, type, restart, output);
		}
	}
	else if (cmd == "replay") {
		std::string file = params.getString("igc");
		std::string type = params.getString("type");
		if (file.empty() || type.empty()) {
			reportFailure("Must supply igc and type parameters to replay", 0, output);
		}
		else {
			replayIgcFile(file, type, output);
		}
	}
	else if (cmd == "clear") {
		clearIgcTraffic( output);
	}
	else if (cmd == "start") {
		int interval = params.getInt("interval", 4);
		std::string p1 = params.getString("p1");
		std::string p2 = params.getString("p2");
		std::string glider_type = params.getString("glider_type");
		std::string registration = params.getString("registration");
		std::string competition_class = params.getString("competition_class");
		std::string competition_id = params.getString("competition_id");

		std::string err;
		if(start(pSim, interval, p1,p2, glider_type, registration, competition_class, competition_id,err)){
			reportSuccess(output);
		}
		else {
			reportFailure(err.c_str(),0,output);
		}
	}
	else if (cmd == "stop") {
		std::string err;
		if(stop(pSim, err)) {
			reportSuccess(output);
		}
		else {
			reportFailure(err.c_str(), 0, output);
		}
	}
	else {
		reportFailure("Unknown igc command", 0, output);
	}
}

bool IgcMessageHandler::listFiles(Simulator* pSim, const std::string& filter, std::list<std::string>& fileList, std::string& err)
{
	File::ListT files;

	try {
		// E:\Users\rbp28668\Documents\Prepar3D v4 Files
		DocumentDirectory documents;
		Directory igcFolder = documents.sub(pSim->documentsFolder()).sub("igc");
		files = igcFolder.files(files, filter + "*.igc");
	}
	catch (FileException& fx) {
		std::ostringstream os;
		os << fx.what() << " (" << fx.err() << ")";
		err = os.str();
		return false;
	}

	for (File::ListT::iterator it = files.begin(); it != files.end(); ++it) {
		fileList.push_back( it->name());
	}

	return true;
}

bool IgcMessageHandler::launch(Simulator* pSim, const std::string& file, const std::string& type, bool restart, std::string& err)
{
	IGCFile igc;

	DocumentDirectory documents;
	Directory igcFolder = documents.sub(pSim->documentsFolder()).sub("igc");
	File f = igcFolder.file(file);

	if (f.exists()) {
		igc.parse((const char*)f);

		AIIGCAircraft* aircraft = new AIIGCAircraft(pSim, &igc, type.c_str());
		aircraft->setRestartOnFinish(restart);
		pSim->externalSim().startAIVehicle(aircraft, "");
		return true;
	}
	else {
		err = "File does not exist";
		return false;
	}
}


bool IgcMessageHandler::replay(Simulator* pSim, const std::string& file, const std::string& type, std::string& err)
{
	IGCFile igc;
	DocumentDirectory documents;
	Directory igcFolder = documents.sub(pSim->documentsFolder()).sub("igc");
	File f = igcFolder.file(file);

	if (f.exists()) {
		igc.parse((const char*)f);

		UserIGCAircraft* aircraft = new UserIGCAircraft(pSim, &igc, type.c_str());
		pSim->externalSim().startCurrentVehicle(aircraft, "");
		return true;
	}
	else {
		err = "File does not exist";
		return false;
	}

}

// Clears all the external sim traffic. Not just IGC.
bool IgcMessageHandler::clear(Simulator* pSim, std::string& err)
{
	pSim->externalSim().clear();
	return true;
}

// Start the flight recorder with the given params
bool IgcMessageHandler::start(Simulator* pSim, int interval, const std::string& p1, const std::string& p2, const std::string& glider_type, const std::string& registration, const std::string& competition_class , const std::string& competition_id, std::string& err)
{
	IGCFlightRecorder* fr = pSim->getFlightRecorder();
	if (!fr->running()) {
		fr->setInterval(interval);
		fr->setP1(p1);
		fr->setP2(p2);
		fr->setGliderType(glider_type);
		fr->setRegistration(registration);
		fr->setCompetitionClass(competition_class);
		fr->setCompetitionId(competition_id);

		// E:\Users\rbp28668\Documents\Prepar3D v4 Files
		DocumentDirectory documents;
		Directory igcFolder = documents.sub(pSim->documentsFolder()).sub("igc");
		fr->start(igcFolder);
		return true;
	}
	else {
		err = "Flight recorder is already running";
		return false;
	}

}

// Clears all the external sim traffic. Not just IGC.
bool IgcMessageHandler::stop(Simulator* pSim, std::string& err)
{
	IGCFlightRecorder* fr = pSim->getFlightRecorder();
	if (fr->running()) {
		fr->stop();
		return true;
	}
	else {
		err = "Flight recorder is not running";
		return false;
	}
}







void IgcMessageHandler::listAvailableIGCFiles( Simulator* pSim, const std::string& filter, std::string& output)
{
	std::list<std::string> fileList;
	std::string err;

	if (listFiles(pSim, filter, fileList, err)) {
		JSONWriter json(output);
		json.add("status", "OK");
		json.array("entries");
		for (auto it = fileList.begin(); it != fileList.end(); ++it) {
			json.object();
			json.add("filename", *it);
			json.end(); // of the igc object
		}
	}
	else {
		reportFailure(err.c_str(), 0, output);
	}
}


void IgcMessageHandler::launchIgcTraffic(const std::string& file, const std::string& type, bool restart, std::string& output)
{
	Simulator* pSim = static_cast<Simulator*>(p3d);
	if(launch(pSim, file, type, restart, output)) {
		reportSuccess(output);
	}
	else {
		reportFailure("File does not exist", 0, output);
	}

}

void IgcMessageHandler::replayIgcFile(const std::string& file, const std::string& type, std::string& output)
{
	Simulator* pSim = static_cast<Simulator*>(p3d);
	std::string err;
	if (replay(pSim, file, type, err)) {
		reportSuccess(output);
	}
	else {
		reportFailure(err.c_str(), 0, output);
	}
}

// Clears all the external sim traffic. Not just IGC.
void IgcMessageHandler::clearIgcTraffic(std::string& output)
{
	Simulator* pSim = static_cast<Simulator*>(p3d);
	std::string err;
	if (clear(pSim, err)) {
		reportSuccess(output);
	}
	else {
		reportFailure(err.c_str(), 0, output);
	}
}

