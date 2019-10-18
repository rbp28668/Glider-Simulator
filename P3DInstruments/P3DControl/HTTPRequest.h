#pragma once

#include "http.h"

// Class to encapsulate the data in a HTTP request.
class HTTPRequest
{
	PHTTP_REQUEST      pRequest;
	PCHAR              pRequestBuffer;
	ULONG              RequestBufferLength;

public:
	HTTP_REQUEST_ID id() const { return pRequest->RequestId; }
	bool hasNullId() const;
	ULONG length() const { return RequestBufferLength; }
	PHTTP_REQUEST rawRequest() const { return pRequest; }

	HTTP_VERB verb() const { return pRequest->Verb; }
	ULONG flags() const { return pRequest->Flags; }

	HTTPRequest(DWORD bufferSize);
	~HTTPRequest();
	void reset();
	void resizeBuffer(DWORD newBufferSize);
};

