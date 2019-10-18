#include "stdafx.h"
#include "HTTPService.h"
#include "HTTPException.h"
#include "http.h"

// Class to manage the lifetime of the basic HTTP service.
// Lifetime of the service object needs to span all the HTTP
// usage.
HTTPService::HTTPService()
{
	HTTPAPI_VERSION HttpApiVersion = HTTPAPI_VERSION_1;
    // Initialize HTTP Server APIs
	ULONG retCode = ::HttpInitialize(
		HttpApiVersion,
		HTTP_INITIALIZE_SERVER,    // Flags
		NULL                       // Reserved
	);

	if (retCode != NO_ERROR)
	{
		throw HTTPException("HttpInitialize failed", retCode);
	}

}

HTTPService::~HTTPService()
{
	::HttpTerminate(HTTP_INITIALIZE_SERVER, NULL);
}
