#pragma once
#include "HTTPRequestQueue.h"

class CommandInterpreter;

// Class to implement a REST API on top of P3D
class RESTAPI :	public HTTPRequestQueue
{
	CommandInterpreter* pInterpreter;
public:
	RESTAPI(CommandInterpreter* pInterpreter);
	virtual ~RESTAPI();
	virtual void onGet(const HTTPRequest& request, HTTPResponse& response);

};

