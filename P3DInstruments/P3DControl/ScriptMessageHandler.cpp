#include "stdafx.h"
#include <sstream>
#include "ScriptMessageHandler.h"
#include "..//P3DCommon/Prepar3D.h"
#include "APIParameters.h"
#include "Folder.h"
#include "JSONWriter.h"
#include "Lua.h"
#include "LuaThread.h"


ScriptMessageHandler::ScriptMessageHandler(Prepar3D* p3d)
	: MessageHandler(p3d, "script")
{

}

ScriptMessageHandler::~ScriptMessageHandler()
{
}

void ScriptMessageHandler::run(const std::string& cmd, const APIParameters& params, std::string& output)
{
	if (cmd == "list") {
		std::string filter = params.getString("filter");
		listScriptFiles(filter, output);
	}
	else if (cmd == "run") {

		std::string file = params.getString("script");
		bool threaded = params.getBool("threaded", true);  // run threaded by default.
		if (file.empty()) {
			reportFailure("Must supply file to run script", 0, output);
		}
		else {
			runScript(file, threaded, output);
		}
	}
	else {
		reportFailure("Unknown igc command", 0, output);
	}
}

void ScriptMessageHandler::listScriptFiles(const std::string& filter, std::string& output)
{
	File::ListT files;

	try {
		// E:\Users\rbp28668\Documents\Prepar3D v4 Files
		DocumentDirectory documents;
		Directory scriptFolder = documents.sub(Prepar3D::DOCUMENTS).sub("script");
		files = scriptFolder.files(files, filter + "*.lua");
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
		json.end(); 
	}

}

void ScriptMessageHandler::runScript(const std::string& file, bool threaded,  std::string& output)
{
	DocumentDirectory documents;
	Directory scriptFolder = documents.sub(Prepar3D::DOCUMENTS).sub("script");
	File f = scriptFolder.file(file);

	if (f.exists()) {

		std::string error;
		int status;

		if (threaded) {
			LuaThread* pThread = new LuaThread((Simulator*)p3d);
			status = pThread->runFile(f, error); // load then run in thread
		}
		else {
			Lua lua((Simulator*)p3d);
			status = lua.runFile(f, error);
		}
		if (status) {
			reportFailure(error.c_str(), 0, output);
		}
		else {
			reportSuccess(output);
		}
	}
	else {
		reportFailure("File does not exist", 0, output);
	}

}

