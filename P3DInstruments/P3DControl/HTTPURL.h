#pragma once
#include <string>

// Class to add a HTTP URL to a request queue.
class HTTPURL
{
	std::wstring theURL;
	HANDLE hReqQueue;

	const char* translateError(ULONG retCode);

public:
	HTTPURL(HANDLE requestQueue, const wchar_t* url);
	~HTTPURL();
};

