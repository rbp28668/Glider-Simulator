#include "stdafx.h"
#include <sstream>
#include <time.h>
#include <iomanip>
#include <assert.h>
#include "Logger.h"
#include "Folder.h"

#include "../P3DCommon/Prepar3D.h"

std::string Logger::getOutputPath(Prepar3D* p3d)
{
	DocumentDirectory documents;
	
	Directory logFolder = (logPath.empty())
		? documents.sub(p3d->documentsFolder()).sub("log")
		: Directory(logPath);

	time_t now;
	time(&now);
	tm t;
	gmtime_s(&t, &now);

	int d = t.tm_mday;
	int m = t.tm_mon + 1;
	int y = t.tm_year + 1900;

	std::ostringstream os;

	os << std::setw(4) << std::setfill('0') << y
		<< std::setw(2) << std::setfill('0') << m
		<< std::setw(2) << std::setfill('0') << d
		<< ".log";

	File file = logFolder.file(os.str());

	return file;
}

void Logger::openOutput(Prepar3D* p3d)
{
	assert(this);
	assert(output == 0);

	std::string path = getOutputPath(p3d);
	output = new std::ofstream(path);
}


// Writes the text string with initial timestamp.
void Logger::write(Prepar3D* p3d, const std::string& text)
{
	time_t now;
	time(&now);
	tm t;
	gmtime_s(&t, &now);

	int d = t.tm_mday;
	int m = t.tm_mon + 1;
	int y = t.tm_year + 1900;
	int hr = t.tm_hour;
	int min = t.tm_min;
	int sec = t.tm_sec;

	if (output == 0) {
		openOutput(p3d);
	}

	(*output) << std::setw(4) << std::setfill('0') << y
		<< std::setw(2) << std::setfill('0') << m
		<< std::setw(2) << std::setfill('0') << d
		<< "T"
		<< std::setw(2) << std::setfill('0') << hr
		<< std::setw(2) << std::setfill('0') << min
		<< std::setw(2) << std::setfill('0') << sec
		<< "Z "
		<< text
		<< std::endl;

	output->flush();
}

Logger::Logger(const char* logPath)
	: output(0)
{
	if (logPath != nullptr) {
		this->logPath = std::string(logPath);
	}

	if (this->logPath.empty()) {
		// Check for the P3D_LOG environment variable.  If it exists and its a folder then use
		// that instead.
		char buffer[512];
		auto size = ::GetEnvironmentVariableA("P3D_LOG", buffer, 512);
		if (size > 0) {
			auto dir = std::string(buffer, size);
			DWORD dwAttrib = GetFileAttributesA(dir.c_str());

			bool isDir = (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
			if (isDir) {
				this->logPath = dir;
			}

		}

	}

}

Logger::~Logger()
{
	if (output) {
		output->close();
	}
}

void Logger::logCommand(Prepar3D* p3d, const std::string& cmd, const std::string& params)
{
	std::string msg = "COMMAND: " + cmd + " " + params;
	write(p3d,msg);
}

void Logger::logSession(Prepar3D* p3d, const std::string& event)
{
	std::string message = "SESSION: " + event;
	write(p3d,message);
}

void Logger::info(Prepar3D* p3d, const std::string& msg)
{
	std::string message = "INFO: " + msg;
	write(p3d,message);
}

void Logger::warn(Prepar3D* p3d, const std::string& msg)
{
	std::string message = "WARN: " + msg;
	write(p3d,message);
}

void Logger::error(Prepar3D* p3d, const std::string& msg)
{
	std::string message = "ERROR: " + msg;
	write(p3d,message);
}
