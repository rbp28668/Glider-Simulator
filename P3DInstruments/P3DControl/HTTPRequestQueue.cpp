#include "stdafx.h"
#include <iostream> 
#include "HTTPRequestQueue.h"
#include "HTTPURL.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "HTTPException.h"
#include "http.h"

// Class to manage an entity buffer for post requests.
// Uses resource allocation is initialisation pattern to ensure cleanup.
class EntityBuffer {
	PUCHAR          pEntityBuffer;
	ULONG           EntityBufferLength;
public:
	PUCHAR bytes() { return pEntityBuffer; }
	ULONG length() { return EntityBufferLength; }

	EntityBuffer(DWORD size);
	~EntityBuffer();
};

EntityBuffer::EntityBuffer(DWORD size) : EntityBufferLength(0), pEntityBuffer(0) {
	EntityBufferLength = size;
	pEntityBuffer = new unsigned char[size];
}

EntityBuffer::~EntityBuffer() {
	if (pEntityBuffer) {
		delete[] pEntityBuffer;
	}
}

// Class to manage a temporary file.
class TempFile {
	HANDLE hTempFile;
	TCHAR szTempName[MAX_PATH + 1];
public:
	HANDLE handle() const { return hTempFile; }
	TempFile();
	~TempFile();

};

TempFile::TempFile() : hTempFile(INVALID_HANDLE_VALUE) {

	DWORD result;

	if (::GetTempFileName(L".", L"New", 0, szTempName) == 0)
	{
		result = ::GetLastError();
		throw result; // TODO properly
	}

	hTempFile = ::CreateFile(
		szTempName,
		GENERIC_READ | GENERIC_WRITE,
		0,                  // Do not share.
		NULL,               // No security descriptor.
		CREATE_ALWAYS,      // Overrwrite existing.
		FILE_ATTRIBUTE_NORMAL,    // Normal file.
		NULL
	);

	if (hTempFile == INVALID_HANDLE_VALUE)
	{
		result = ::GetLastError();
		throw HTTPException("Cannot create temporary file", result);
	}

}

TempFile::~TempFile() {
	if (hTempFile != INVALID_HANDLE_VALUE) {
		::CloseHandle(hTempFile);
		::DeleteFile(szTempName);

	}
}


// Main request queue.

HTTPRequestQueue::HTTPRequestQueue()
	: hReqQueue(0)
{
	ULONG retCode = ::HttpCreateHttpHandle(
		&hReqQueue,        // Req Queue
		0                  // Reserved
	);

	if (retCode != NO_ERROR) {
		throw HTTPException("Unable to create queue handle",retCode);
	}
}

HTTPRequestQueue::~HTTPRequestQueue()
{
	while (!urls.empty()) {
		HTTPURL* pURL = urls.front();
		urls.pop_front();
		delete pURL;
	}

	if (hReqQueue) {
		::CloseHandle(hReqQueue);
	}
}

void HTTPRequestQueue::addURL(const wchar_t* url)
{
	HTTPURL* pUrl = new HTTPURL(hReqQueue, url);
	urls.push_back(pUrl);
}

ULONG HTTPRequestQueue::receiveRequests()
{
	ULONG              result;
	DWORD              bytesRead;

	// Create an initial HTTP request with a buffer size of 2k.
	HTTPRequest request(4096);

	for (;;)
	{
		request.reset();
		std::cout << "Initialised request ID " << request.id() << std::endl;

		std::cout << "Receiving from queue handle " << hReqQueue << std::endl;

		HTTP_REQUEST_ID id = request.id();
		result = ::HttpReceiveHttpRequest(
			hReqQueue,          // Req Queue
			id,                 // Req ID
			0,                  // Flags
			request.rawRequest(),           // HTTP request buffer
			request.length(),   // req buffer length
			&bytesRead,         // bytes received
			NULL                // LPOVERLAPPED
		);

		std::cout << "Received " << bytesRead << " bytes with request ID " << request.id() << std::endl;

		if (NO_ERROR == result)
		{
			//
			// Worked! 
			// 
			HTTPResponse response;

			switch (request.verb())
			{
			case HttpVerbGET:
				onGet(request, response);
				result = sendResponse(request, response);
				break;

			case HttpVerbPOST:
				onPost(request, response);
				result = sendPostResponse(request, response);
				break;

			default:
				response.setStatus(503, "Not Implemented");
				result = sendResponse(request, response);
				break;
			}

			if (result != NO_ERROR)
			{
				break;
			}

			//
			// Reset the Request ID to handle the next request.
			//
			request.reset();
		}
		else if (result == ERROR_MORE_DATA)
		{
			//
			// The input buffer was too small to hold the request
			// headers. Increase the buffer size and call the 
			// API again. 
			//
			// When calling the API again, handle the request
			// that failed by passing a RequestID.
			//
			// This RequestID is read from the old buffer.
			//
			request.resizeBuffer(bytesRead);

		}
		else if (ERROR_CONNECTION_INVALID == result && !request.hasNullId())
		{
			// The TCP connection was corrupted by the peer when
			// attempting to handle a request with more buffer. 
			// Continue to the next request.
			request.reset();
		}
		else
		{
			break;
		}

	}


	return result;
}

void HTTPRequestQueue::onGet(const HTTPRequest& request, HTTPResponse& response)
{
	response.setStatus(503, "Not Implemented");
}

void HTTPRequestQueue::onPost(const HTTPRequest& request, HTTPResponse& response)
{
	response.setStatus(503, "Not Implemented");
}


ULONG HTTPRequestQueue::sendResponse(HTTPRequest& request, HTTPResponse& response) {

	DWORD bytesSent;
	ULONG result;

	// 
	// Because the entity body is sent in one call, it is not
	// required to specify the Content-Length.
	//

	//std::cout << "Sending response to queue handle " << hReqQueue << std::endl;

	HTTP_REQUEST_ID id = request.id();

	result = ::HttpSendHttpResponse(
		hReqQueue,           // ReqQueueHandle
		id,					 // Request ID
		0,                   // Flags
		response.rawResponse(), // HTTP response
		NULL,                // pReserved1
		&bytesSent,          // bytes sent  (OPTIONAL)
		NULL,                // pReserved2  (must be NULL)
		0,                   // Reserved3   (must be 0)
		NULL,                // LPOVERLAPPED(OPTIONAL)
		NULL                 // pReserved4  (must be NULL)
	);

	if (result != NO_ERROR)
	{
		wprintf(L"HttpSendHttpResponse failed with %lu \n", result);
	}

	return result;
}

#define MAX_ULONG_STR ((ULONG) sizeof("4294967295"))


ULONG HTTPRequestQueue::sendPostResponse(HTTPRequest& request, HTTPResponse& response)
{
	DWORD           result;
	DWORD           bytesSent;
	ULONG           BytesRead;
	ULONG           TempFileBytesWritten;
	CHAR            szContentLength[MAX_ULONG_STR];
	HTTP_DATA_CHUNK dataChunk;
	ULONG           TotalBytesRead = 0;

	BytesRead = 0;

	//
	// Allocate space for an entity buffer. Buffer can be increased 
	// on demand.
	//
	EntityBuffer entityBuffer(2048);

	//
	// For POST, echo back the entity from the
	// client
	//
	// NOTE: If the HTTP_RECEIVE_REQUEST_FLAG_COPY_BODY flag had been
	//       passed with HttpReceiveHttpRequest(), the entity would 
	//       have been a part of HTTP_REQUEST (using the pEntityChunks
	//       field). Because that flag was not passed, there are no
	//       o entity bodies in HTTP_REQUEST.
	//

	if (request.flags() & HTTP_REQUEST_FLAG_MORE_ENTITY_BODY_EXISTS)
	{
		// The entity body is sent over multiple calls. Collect 
		// these in a file and send back. Create a temporary 
		// file.
		//

		TempFile tempFile;

		bool finished = false;
		do
		{
			//
			// Read the entity chunk from the request.
			//
			BytesRead = 0;
			result = HttpReceiveRequestEntityBody(
				hReqQueue,
				request.id(),
				0,
				entityBuffer.bytes(),
				entityBuffer.length(),
				&BytesRead,
				NULL
			);

			switch (result)
			{
			case NO_ERROR:

				if (BytesRead != 0)
				{
					TotalBytesRead += BytesRead;
					WriteFile(
						tempFile.handle(),
						entityBuffer.bytes(),
						BytesRead,
						&TempFileBytesWritten,
						NULL
					);
				}
				break;

			case ERROR_HANDLE_EOF:

				//
				// The last request entity body has been read.
				// Send back a response. 
				//
				// To illustrate entity sends via 
				// HttpSendResponseEntityBody, the response will 
				// be sent over multiple calls. To do this,
				// pass the HTTP_SEND_RESPONSE_FLAG_MORE_DATA
				// flag.

				if (BytesRead != 0)
				{
					TotalBytesRead += BytesRead;
					WriteFile(
						tempFile.handle(),
						entityBuffer.bytes(),
						BytesRead,
						&TempFileBytesWritten,
						NULL
					);
				}

				//
				// Because the response is sent over multiple
				// API calls, add a content-length.
				//
				// Alternatively, the response could have been
				// sent using chunked transfer encoding, by  
				// passimg "Transfer-Encoding: Chunked".
				//

				// NOTE: Because the TotalBytesread in a ULONG
				//       are accumulated, this will not work
				//       for entity bodies larger than 4 GB. 
				//       For support of large entity bodies,
				//       use a ULONGLONG.
				// 


				sprintf_s(szContentLength, MAX_ULONG_STR, "%lu", TotalBytesRead);
				response.addHeader(HttpHeaderContentLength, szContentLength);

				result =
					HttpSendHttpResponse(
						hReqQueue,           // ReqQueueHandle
						request.id(), // Request ID
						HTTP_SEND_RESPONSE_FLAG_MORE_DATA,
						response.rawResponse(),   // HTTP response
						NULL,            // pReserved1
						&bytesSent,      // bytes sent-optional
						NULL,            // pReserved2
						0,               // Reserved3
						NULL,            // LPOVERLAPPED
						NULL             // pReserved4
					);

				if (result != NO_ERROR)
				{
					wprintf(L"HttpSendHttpResponse failed with %lu \n", result);
					finished = true;
				}

				//
				// Send entity body from a file handle.
				//
				dataChunk.DataChunkType = HttpDataChunkFromFileHandle;
				dataChunk.FromFileHandle.ByteRange.StartingOffset.QuadPart = 0;
				dataChunk.FromFileHandle.ByteRange.Length.QuadPart = HTTP_BYTE_RANGE_TO_EOF;
				dataChunk.FromFileHandle.FileHandle = tempFile.handle();

				result = ::HttpSendResponseEntityBody(
					hReqQueue,
					request.id(),
					0,           // This is the last send.
					1,           // Entity Chunk Count.
					&dataChunk,
					NULL,
					NULL,
					0,
					NULL,
					NULL
				);

				if (result != NO_ERROR)
				{
					wprintf(L"HttpSendResponseEntityBody failed %lu\n", result);
					finished = true;
				}


				break;


			default:
				wprintf(L"HttpReceiveRequestEntityBody failed with %lu \n", result);
				finished = true;
			}

		} while (!finished);
	}
	else
	{
		// This request does not have an entity body.
		//

		result = HttpSendHttpResponse(
			hReqQueue,           // ReqQueueHandle
			request.id(), // Request ID
			0,
			response.rawResponse(), // HTTP response
			NULL,                // pReserved1
			&bytesSent,          // bytes sent (optional)
			NULL,                // pReserved2
			0,                   // Reserved3
			NULL,                // LPOVERLAPPED
			NULL                 // pReserved4
		);
		if (result != NO_ERROR)
		{
			wprintf(L"HttpSendHttpResponse failed with %lu \n",
				result);
		}
	}

	return result;
}


