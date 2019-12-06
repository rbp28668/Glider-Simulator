#pragma once
#include <string>
#include <fstream>
class Logger
{
	std::ofstream* output;

	std::string getOutputPath();
	void openOutput();
	void write(const std::string& text);

public:
	Logger();
	~Logger();

	void logCommand(const std::string& cmd, const std::string& params);
	void logSession(const std::string& event);
	void info(const std::string& msg);
	void warn(const std::string& msg);
	void error(const std::string& msg);
};

