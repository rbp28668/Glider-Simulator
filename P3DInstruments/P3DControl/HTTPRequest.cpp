#include "stdafx.h"
#include "HTTPRequest.h"


HTTPRequest::HTTPRequest(DWORD bufferSize) 
: pRequestBuffer(0)
{
   // Allocate a 2 KB buffer. This size should work for most 
   // requests. The buffer size can be increased if required. Space
   // is also required for an HTTP_REQUEST structure.
	RequestBufferLength = sizeof(HTTP_REQUEST) + 2048;
	pRequestBuffer = new char[RequestBufferLength];

	pRequest = (PHTTP_REQUEST)pRequestBuffer;

	reset();
}


HTTPRequest::~HTTPRequest() {
	if (pRequestBuffer) {
		delete [] pRequestBuffer;
    }
}

bool HTTPRequest::hasNullId() const {
	return HTTP_IS_NULL_ID(&pRequest->RequestId);
}

void HTTPRequest::reset() {
	// Set to wait for a new request.
	RtlZeroMemory(pRequest, RequestBufferLength);
}

void HTTPRequest::resizeBuffer(DWORD newBufferSize) {
	HTTP_REQUEST_ID requestId = pRequest->RequestId;  // retrieve the request ID before we kill the old buffer.

	RequestBufferLength = newBufferSize;
	delete[] pRequestBuffer;
	pRequestBuffer = new char [RequestBufferLength];

	pRequest = (PHTTP_REQUEST)pRequestBuffer;
	pRequest->RequestId = requestId;
}

