#include "stdafx.h"
#include "MessageHandler.h"
#include "JSONWriter.h"
#include <sstream>

MessageHandler::MessageHandler(Prepar3D* p3d, const std::string& name)
	: p3d(p3d), name(name)
{
}

void MessageHandler::reportSuccess(std::string& output)
{
	JSONWriter json(output);
	json.add("status", "OK");
}

void MessageHandler::reportFailure(const char* pszReason, unsigned long code, std::string& output)
{
	JSONWriter json(output);
	json.add("status", "FAILED");
	std::ostringstream os;
	os << pszReason;
	if (code != 0) {
		os << " (" << code << ")";
	}
	json.add("reason", os.str());
}
