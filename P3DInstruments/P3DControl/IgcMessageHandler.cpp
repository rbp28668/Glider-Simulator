#include "stdafx.h"
#include <sstream>
#include "..//P3DCommon/Prepar3D.h"
#include "..//P3DCommon/ExternalSim.h"
#include "IgcMessageHandler.h"
#include "APIParameters.h"
#include "IGCFile.h"
#include "IGCAircraft.h"
#include "Folder.h"
#include "JSONWriter.h"

IgcMessageHandler::IgcMessageHandler(Prepar3D* p3d) 
	: MessageHandler(p3d, "igc")
	, fr(p3d)
{

}

IgcMessageHandler::~IgcMessageHandler()
{
}

void IgcMessageHandler::run(const std::string& cmd, const APIParameters& params, std::string& output)
{
	if (cmd == "list") {
		std::string filter = params.getString("filter");
		listAvailableIGCFiles(filter, output);
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
		clearIgcTraffic(output);
	}
	else if (cmd == "start") {
		if (!fr.running()) {
			fr.setInterval(params.getInt("interval", 4));
			fr.setP1(params.getString("p1"));
			fr.setP2(params.getString("p2"));
			fr.setGliderType(params.getString("glider_type"));
			fr.setRegistration(params.getString("registration"));
			fr.setCompetitionClass(params.getString("competition_class"));
			fr.setCompetitionId(params.getString("competition_id"));

			// E:\Users\rbp28668\Documents\Prepar3D v4 Files
			DocumentDirectory documents;
			Directory igcFolder = documents.sub(Prepar3D::DOCUMENTS).sub("igc");
			fr.start(igcFolder);
			reportSuccess(output);
		}
		else {
			reportFailure("Flight recorder is already running",0,output);
		}

	}
	else if (cmd == "stop") {
		if (fr.running()) {
			fr.stop();
			reportSuccess(output);
		}
		else {
			reportFailure("Flight recorder is not running", 0, output);
		}
	}
	else {
		reportFailure("Unknown igc command", 0, output);
	}


}

void IgcMessageHandler::listAvailableIGCFiles(const std::string& filter, std::string& output)
{
	File::ListT files;

	try {
		// E:\Users\rbp28668\Documents\Prepar3D v4 Files
		DocumentDirectory documents;
		Directory igcFolder = documents.sub(Prepar3D::DOCUMENTS).sub("igc");
		files = igcFolder.files(files, filter + "*.igc");
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
		json.object();
		json.add("filename", it->name());
		json.end(); // of the scenario object
	}

	//reportSuccess(output);
}

void IgcMessageHandler::launchIgcTraffic(const std::string& file, const std::string& type, bool restart, std::string& output)
{
	IGCFile igc;

	DocumentDirectory documents;
	Directory igcFolder = documents.sub(Prepar3D::DOCUMENTS).sub("igc");
	File f = igcFolder.file(file);

	if (f.exists()) {
		igc.parse((const char*)f);

		AIIGCAircraft* aircraft = new AIIGCAircraft(p3d, &igc, type.c_str());
		aircraft->setRestartOnFinish(restart);
		p3d->externalSim().startAIVehicle(aircraft, "");
		reportSuccess(output);
	}
	else {
		reportFailure("File does not exist", 0, output);
	}

}

void IgcMessageHandler::replayIgcFile(const std::string& file, const std::string& type, std::string& output)
{
	IGCFile igc;
	DocumentDirectory documents;
	Directory igcFolder = documents.sub(Prepar3D::DOCUMENTS).sub("igc");
	File f = igcFolder.file(file);

	if (f.exists()) {
		igc.parse((const char*)f);

		UserIGCAircraft* aircraft = new UserIGCAircraft(p3d, &igc, type.c_str());
		p3d->externalSim().startCurrentVehicle(aircraft, "");
		reportSuccess(output);
	}
	else {
		reportFailure("File does not exist", 0, output);
	}

}

// Clears all the external sim traffic. Not just IGC.
void IgcMessageHandler::clearIgcTraffic(std::string& output)
{
	p3d->externalSim().clear();
	reportSuccess(output);
}

