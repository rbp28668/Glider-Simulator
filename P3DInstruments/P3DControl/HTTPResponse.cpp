#include "stdafx.h"
#include <string.h>
#include "HTTPResponse.h"

HTTPResponse::HTTPResponse() : pszEntity(0)
{
	RtlZeroMemory(&response, sizeof(response));
}

HTTPResponse::~HTTPResponse()
{
	if (pszEntity) {
		delete[] pszEntity;
	}
}

// WARNING- keeps a reference to the reason string
void HTTPResponse::setStatus(short statusCode, const char* reason) {
	response.StatusCode = statusCode;  
	response.pReason = (reason);   
	response.ReasonLength = (USHORT)strlen(reason); 
}

// WARNING- keeps a reference to the header value
void HTTPResponse::addHeader(HTTP_HEADER_ID id, const char* value)
{
	response.Headers.KnownHeaders[id].pRawValue = value;
	response.Headers.KnownHeaders[id].RawValueLength = (USHORT)strlen(value);                               \
}

void HTTPResponse::setEntityString(const char* value)
{
	if (pszEntity) {
		delete[] pszEntity;
	}

	// Note managing a buffer by hand as not guaranteed to be const.
	size_t len = strlen(value) + 1;
	pszEntity = new char[len];
	strncpy_s(pszEntity, len, value, len);
	pszEntity[len - 1] = 0; // ensure terminated.

	// Add an entity chunk.
	dataChunk.DataChunkType = HttpDataChunkFromMemory;
	dataChunk.FromMemory.pBuffer = static_cast<PVOID>(pszEntity);
	dataChunk.FromMemory.BufferLength = (ULONG)strlen(pszEntity);

	response.EntityChunkCount = 1;
	response.pEntityChunks = &dataChunk;
}
