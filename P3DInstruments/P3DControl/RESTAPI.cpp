#include "stdafx.h"
#include <iostream>
#include "RESTAPI.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "CommandInterpreter.h"
#include "APIParameters.h"

RESTAPI::RESTAPI(CommandInterpreter* pInterpreter) 
	: pInterpreter(pInterpreter)
{
	addURL(L"http://+:80/p3dapi/");
}

RESTAPI::~RESTAPI()
{
}

std::string ws2s(const std::wstring& s)
{
	int slength = (int)s.length();

	// How many chars needed?
	int len = ::WideCharToMultiByte(CP_UTF8, 0, s.c_str(), slength, 0, 0, 0, 0);

	std::string r(len, '\0'); // empty string of just the right size

	// Transform into r.
	::WideCharToMultiByte(CP_UTF8, 0, s.c_str(), slength, &r[0], len, 0, 0);
	return r;
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
	response.addHeader(HttpHeaderContentType, "text/html");
	response.setEntityString(output.c_str());
}
