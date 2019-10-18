#pragma once
#include <list>

class HTTPURL;
class HTTPRequest;
class HTTPResponse;

// Class to encapsulate an HTTP request queue.  This is effectively a
// web service module and can listen on one or mor URLs.  Over-ride
// onGet or onPost as necessary. (Post largely demo code).
class HTTPRequestQueue
{
	HANDLE          hReqQueue;
	std::list<HTTPURL*> urls;

	ULONG sendResponse(HTTPRequest& request, HTTPResponse& response);
	ULONG sendPostResponse(HTTPRequest& request, HTTPResponse& response);

public:
	HTTPRequestQueue();
	virtual ~HTTPRequestQueue();
	HANDLE handle() const { return hReqQueue; }
	void addURL(const wchar_t* url);
	ULONG receiveRequests();
	virtual void onGet(const HTTPRequest& request, HTTPResponse& response);
	virtual void onPost(const HTTPRequest& request, HTTPResponse& response);
};

