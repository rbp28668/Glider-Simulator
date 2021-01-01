#include "stdafx.h"
#include "..//P3DCommon/Prepar3D.h"
#include "RecordMessageHandler.h"
#include "APIParameters.h"
#include "Folder.h"
#include "JSONWriter.h"

RecordMessageHandler::RecordMessageHandler(Prepar3D* p3d)
	: MessageHandler(p3d, "record")
{
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
	HRESULT hr = ::SimConnect_GenerateFlightAnalysisDiagrams(
		p3d->getHandle()
	);
	if (hr == S_OK) {
		reportSuccess(output);
	}
	else {
		reportFailure("Unable to start recording", hr, output);
	}
}

void RecordMessageHandler::listRecordings(std::string& output) {
	try {
		DocumentDirectory docs;
		Directory recordingFolder = docs.sub("Prepar3D v4 files");

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
	HRESULT hr = ::SimConnect_StartRecorder(
		p3d->getHandle()
	);

	if (hr == S_OK) {
		reportSuccess(output);
	}
	else {
		reportFailure("Unable to start recording", hr, output);
	}
}

void RecordMessageHandler::stopRecordingAndSave(const std::string& title, const std::string& description, std::string& output)
{
	HRESULT hr = ::SimConnect_StopRecorderAndSaveRecording(
		p3d->getHandle(),
		title.c_str(),
		description.c_str(),
		FALSE // don't propmt the user
	);
	if (hr == S_OK) {
		reportSuccess(output);
	}
	else {
		reportFailure("Unable to stop and save recording", hr, output);
	}

}

void RecordMessageHandler::playbackRecording(const std::string& name, std::string& output)
{

	//Expand name to full path.
	DocumentDirectory docs;
	Directory recordingFolder = docs.sub("Prepar3D v4 files");
	File file = recordingFolder.file(name.c_str());

	HRESULT hr = SimConnect_PlaybackRecording(
		p3d->getHandle(),
		(const char*)file,
		0,  // from start
		-1, // whole recording
		FALSE // no dialog
	);
	if (hr == S_OK) {
		reportSuccess(output);
	}
	else {
		reportFailure("Unable to playback recording", hr, output);
	}

}


