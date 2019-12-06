#include "stdafx.h"
#include <sstream>
#include <time.h>
#include <iomanip>
#include <assert.h>
#include "Logger.h"
#include "Folder.h"

#include "../P3DCommon/Prepar3D.h"

std::string Logger::getOutputPath()
{
	DocumentDirectory documents;
	Directory logFolder = documents.sub(Prepar3D::DOCUMENTS).sub("log");

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

void Logger::openOutput()
{
	assert(this);
	assert(output == 0);

	std::string path = getOutputPath();
	output = new std::ofstream(path);
}


// Writes the text string with initial timestamp.
void Logger::write(const std::string& text)
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
		openOutput();
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

Logger::Logger()
	: output(0)
{
}

Logger::~Logger()
{
	if (output) {
		output->close();
	}
}

void Logger::logCommand(const std::string& cmd, const std::string& params)
{
	std::string msg = "COMMAND: " + cmd + " " + params;
	write(msg);
}

void Logger::logSession(const std::string& event)
{
	std::string message = "SESSION: " + event;
	write(message);
}

void Logger::info(const std::string& msg)
{
	std::string message = "INFO: " + msg;
	write(message);
}

void Logger::warn(const std::string& msg)
{
	std::string message = "WARN: " + msg;
	write(message);
}

void Logger::error(const std::string& msg)
{
	std::string message = "ERROR: " + msg;
	write(message);
}
