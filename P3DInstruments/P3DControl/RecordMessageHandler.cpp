#include "stdafx.h"
#include <sstream>
#include "..//P3DCommon/Prepar3D.h"
#include "RecordMessageHandler.h"
#include "APIParameters.h"
#include "Folder.h"
#include "JSONWriter.h"
#include "Simulator.h"

RecordMessageHandler::RecordMessageHandler(Prepar3D* p3d)
	: MessageHandler(p3d, "record")
{
}

std::string RecordMessageHandler::getSubFolder(Prepar3D* pSim) {
	std::stringstream ss;
	ss << "Prepar3D v" << pSim->getMajorVersion() << " files";
	std::string subFolder = ss.str();
	return subFolder;
}

bool RecordMessageHandler::start(Simulator* pSim)
{
	HRESULT hr = ::SimConnect_StartRecorder(
		pSim->getHandle()
	);

	return (hr == S_OK);
}

bool RecordMessageHandler::stop(Simulator* pSim, const std::string& title, const std::string& description)
{
	HRESULT hr = ::SimConnect_StopRecorderAndSaveRecording(
		pSim->getHandle(),
		title.c_str(),
		description.c_str(),
		FALSE // don't propmt the user
	);
	return (hr == S_OK);
}

bool RecordMessageHandler::playback(Simulator* pSim, const std::string& name)
{
	//Expand name to full path.
	DocumentDirectory docs;
	Directory recordingFolder = docs.sub(getSubFolder(pSim));  //was  "Prepar3D v4 files"
	File file = recordingFolder.file(name.c_str());

	HRESULT hr = SimConnect_PlaybackRecording(
		pSim->getHandle(),
		(const char*)file,
		0,  // from start
		-1, // whole recording
		FALSE // no dialog
	);
	return (hr == S_OK);
}

bool RecordMessageHandler::analyse(Simulator* pSim)
{
	HRESULT hr = ::SimConnect_GenerateFlightAnalysisDiagrams(
		pSim->getHandle()
	);
	return(hr == S_OK);

}

File::ListT& RecordMessageHandler::list(Simulator* pSim, File::ListT& files)
{
	DocumentDirectory docs;
	Directory recordingFolder = docs.sub(getSubFolder(pSim)); // was "Prepar3D v4 files"

	recordingFolder.files(files, "*.fsr");

	return files;
}


void RecordMessageHandler::run(const std::string& cmd, const APIParameters& params, std::string& output)
{
	if (cmd == "record") {
		startRecording(output);
	}
	else if (cmd == "play") {
		std::string title = params.getString("title");
		if (title.empty()) {
			reportFailure("No title given for playback", 0, output);
		}
		else {
			playbackRecording(title, output);
		}
	}
	else if (cmd == "stop") {
		std::string title = params.getString("title");
		std::string description = params.getString("description");
		if (title.empty()) {
			reportFailure("No title given for recording", 0, output);
		}
		else {
			stopRecordingAndSave(title, description, output);
		}

	}
	else if (cmd == "analyse") {
		saveAnalysis(output);
	}
	else if (cmd == "list") {
		listRecordings(output);
	}
	else {
		reportFailure("Unknown record command", 0, output);
	}

}

void RecordMessageHandler::saveAnalysis(std::string& output) {
	Simulator* pSim = static_cast<Simulator*>(p3d);
	
	if(analyse(pSim)){
		reportSuccess(output);
	}
	else {
		reportFailure("Unable to start recording", 0, output);
	}
}

void RecordMessageHandler::listRecordings(std::string& output) {
	try {
		DocumentDirectory docs;
		Directory recordingFolder = docs.sub(getSubFolder(p3d)); // "Prepar3D v4 files"

		File::ListT files;
		recordingFolder.files(files, "*.fsr");

		JSONWriter json(output);
		json.add("status", "OK");
		json.array("recordings");

		for (File::ListT::iterator it = files.begin();
			it != files.end();
			++it) {
			json.object();
			json.add("recording", it->name());
			json.end();

		}
		json.end(); // of array
	}
	catch (FileException& fx) {
		output.erase();
		reportFailure(fx.what(), fx.err(), output);
	}
}


void RecordMessageHandler::startRecording(std::string& output)
{
	Simulator* pSim = static_cast<Simulator*>(p3d);
	if(start(pSim)) {
		reportSuccess(output);
	}
	else {
		reportFailure("Unable to start recording", 0, output);
	}
}

void RecordMessageHandler::stopRecordingAndSave(const std::string& title, const std::string& description, std::string& output)
{
	Simulator* pSim = static_cast<Simulator*>(p3d);
	if(stop(pSim, title, description)) {
		reportSuccess(output);
	}
	else {
		reportFailure("Unable to stop and save recording", 0, output);
	}

}

void RecordMessageHandler::playbackRecording(const std::string& name, std::string& output)
{
	Simulator* pSim = static_cast<Simulator*>(p3d);
	if (playback(pSim, name)) {
		reportSuccess(output);
	}
	else {
		reportFailure("Unable to playback recording", 0, output);
	}

}


