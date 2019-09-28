#include "stdafx.h"
#include <assert.h>
#include<iostream>
#include "EventMessageHandler.h"
#include "APIParameters.h"
#include "JSONWriter.h"




EventMessageHandler::EventMessageHandler(Prepar3D * p3d)
	: MessageHandler(p3d, "cmd")
	, commands(p3d)
{
	assert(p3d != 0);
	this->p3d = p3d;
}

EventMessageHandler::~EventMessageHandler()
{
}


void EventMessageHandler::run(const std::string& cmd, const APIParameters& params, std::string& output)
{
	DWORD dwData = params.getDWORD("value",0);
	
	// Force upper case for lookup.
	std::string asUpper(cmd);
	for (std::string::iterator i = asUpper.begin(); i != asUpper.end(); ++i) {
		*i = ::toupper(*i);
	}

	bool failed = commands.dispatchEvent(asUpper, dwData);

	JSONWriter json(output);
	if (failed) {
		json.add("status", "FAILED")
			.add("reason", "Unable to run P3D command " + asUpper);
	} else {
		json.add("status", "OK");
	}
}
