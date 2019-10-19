#pragma once
#include "http.h"
#include <vector>

// Class to provide encapsulated mechanism for sending HTTP response.
class HTTPResponse
{
	HTTP_RESPONSE   response;
	HTTP_DATA_CHUNK dataChunk;
	char* pszEntity;
	std::vector< HTTP_UNKNOWN_HEADER> unknownHeaders;

public:
	HTTPResponse();
	~HTTPResponse();

	HTTP_RESPONSE* rawResponse() { return &response; }

	void setStatus(short statusCode, const char* reason);
	void addHeader(HTTP_HEADER_ID id, const char* value);
	void addHeader(const char* name, const char* value);
	void setEntityString(const char* value);
};


