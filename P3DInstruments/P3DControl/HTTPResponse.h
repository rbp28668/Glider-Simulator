#pragma once
#include "http.h"

// Class to provide encapsulated mechanism for sending HTTP response.
class HTTPResponse
{
	HTTP_RESPONSE   response;
	HTTP_DATA_CHUNK dataChunk;
	char* pszEntity;

public:
	HTTPResponse();
	~HTTPResponse();

	HTTP_RESPONSE* rawResponse() { return &response; }

	void setStatus(short statusCode, const char* reason);
	void addHeader(HTTP_HEADER_ID id, const char* value);
	void setEntityString(const char* value);
};


