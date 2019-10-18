#include "stdafx.h"
#include "HTTPURL.h"
#include "http.h"
#include "HTTPRequestQueue.h"
#include "HTTPException.h"


// See https://docs.microsoft.com/en-gb/windows/win32/http/urlprefix-strings for valid URLs
// Also see https://docs.microsoft.com/en-gb/windows/win32/http/namespace-reservations-registrations-and-routing?redirectedfrom=MSDN,
// and https://docs.microsoft.com/en-gb/windows/win32/http/accessing-the-reservation-store
// Need similar to:
// PS C:\WINDOWS\system32> netsh http add urlacl url=http://+:80/p3dapi user=rbp28668
// URL reservation successfully added

HTTPURL::HTTPURL(HANDLE hReqQueue, const wchar_t* url)
{
	this->hReqQueue = hReqQueue;

	ULONG retCode = ::HttpAddUrl(
		hReqQueue,    // Req Queue
		url,      // Fully qualified URL
		NULL          // Reserved
	);

	if (retCode != NO_ERROR)
	{
		wprintf(L"HttpAddUrl failed with %lu \n", retCode);
		printf("%s\n", translateError(retCode));

		throw HTTPException("Unable to add URL for HTTP service", retCode);
	}

	theURL = std::wstring(url);
}

HTTPURL::~HTTPURL()
{
	::HttpRemoveUrl(
		hReqQueue,         // Req Queue
		theURL.c_str()     // Fully qualified URL
	);

}

const char* HTTPURL::translateError(ULONG retCode) {
	switch (retCode) {
	case ERROR_ACCESS_DENIED: return "The calling application does not have permission to register the URL.";
	case ERROR_DLL_INIT_FAILED: return "The calling application did not call HttpInitialize before calling this function.";
	case ERROR_INVALID_PARAMETER: return "One of the parameters are invalid.";
	case ERROR_ALREADY_EXISTS: return "The specified UrlPrefix conflicts with an existing registration.";
	case ERROR_NOT_ENOUGH_MEMORY: return "Insufficient resources to complete the operation.";
	}

	return "Unknown error setting URL";

}