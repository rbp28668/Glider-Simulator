#pragma once
#include <string>

class APIParameters;
class Prepar3D;

// Class to handle a particular group of messages in the API.
class MessageHandler {
	std::string name;

protected:
	Prepar3D* p3d;
public:
	MessageHandler(Prepar3D* p3d, const std::string& name);
	const std::string& getName() { return name; }
	static void reportSuccess(std::string& output);
	static void reportFailure(const char* pszReason, unsigned long code, std::string& output);

	virtual void run(const std::string& cmd, const APIParameters& params, std::string& output) = 0;

};

