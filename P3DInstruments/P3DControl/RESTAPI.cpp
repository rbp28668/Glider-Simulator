#include "stdafx.h"
#include <iostream>
#include "RESTAPI.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "CommandInterpreter.h"
#include "APIParameters.h"
#include "WideConverter.h"

RESTAPI::RESTAPI(CommandInterpreter* pInterpreter) 
	: pInterpreter(pInterpreter)
{
	addURL(L"http://+:80/p3dapi/");
}

RESTAPI::~RESTAPI()
{
}


void RESTAPI::onGet(const HTTPRequest& request, HTTPResponse& response)
{
	//Received 1517 bytes with request ID 18230571293743251473
	// Full URL : http://localhost:80/p3dapi/cmd/KOHLSMAN_INC?value=1013
	// Abs path : / p3dapi / cmd / KOHLSMAN_INC ? value = 1013
	// Processing cmd / KOHLSMAN_INC ? value = 1013 : ? value = 1013

	std::wcout << "Full URL: " << request.rawRequest()->CookedUrl.pFullUrl << std::endl;
	std::wcout << "Abs path: " << request.rawRequest()->CookedUrl.pAbsPath << std::endl;
	//std::wcout << "Query string: " << request.rawRequest()->CookedUrl.pQueryString << std::endl;


	// Command is the path without the intiial /p3dapi/ (or whatever is in the first /.../)
	std::string cmd = ws2s(request.rawRequest()->CookedUrl.pAbsPath);
	size_t pos = cmd.find_first_of('/');
	pos = cmd.find_first_of('/', pos + 1);
	cmd = cmd.substr(pos+1);

	// Now look for query string and get rid of it from command
	pos = cmd.find_first_of('?');
	if (pos != std::string::npos) {
		cmd = cmd.substr(0, pos);
	}

	std::string params;
	PCWSTR query = request.rawRequest()->CookedUrl.pQueryString;
	if (query) {
		params = ws2s(query);
	}

	std::string output;
	pInterpreter->process(cmd, params, output);

	response.setStatus(200, "OK");
	response.addHeader(HttpHeaderContentType, "application/json");
	response.addHeader("Access-Control-Allow-Origin", "*"); // CORS - see https://developer.mozilla.org/en-US/docs/Web/HTTP/CORS
	response.setEntityString(output.c_str());
}
