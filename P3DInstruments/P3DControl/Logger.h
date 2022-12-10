#pragma once
#include <string>
#include <fstream>
class Prepar3D;


class Logger
{
	std::ofstream* output;

	std::string getOutputPath(Prepar3D* p3d);
	void openOutput(Prepar3D* p3d);
	void write(Prepar3D* p3d, const std::string& text);

public:
	Logger();
	~Logger();

	void logCommand(Prepar3D* p3d,const std::string& cmd, const std::string& params);
	void logSession(Prepar3D* p3d, const std::string& event);
	void info(Prepar3D* p3d, const std::string& msg);
	void warn(Prepar3D* p3d, const std::string& msg);
	void error(Prepar3D* p3d, const std::string& msg);
};

