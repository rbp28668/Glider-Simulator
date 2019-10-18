#pragma once

// see https://docs.microsoft.com/en-us/windows/win32/http/http-server-sample-application

// Simple class to initialise and terminate the HTTP service.  Note that an instance of this should
// live for as long as the service is needed.
class HTTPService
{
public:
	HTTPService();
	~HTTPService();
};

