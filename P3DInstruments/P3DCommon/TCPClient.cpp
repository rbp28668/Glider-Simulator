#include "StdAfx.h"
#include <Ws2tcpip.h>
#include <iostream>
#include <assert.h>
#include "TCPClient.h"


// Connect to the endpoint.  Assumes that winsock initialised.
// Returns true if there is a valid connection.
bool TCPClient::connect() {

	//create socket
	if ( (socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == SOCKET_ERROR)  {
		std::cerr << "socket() failed with error code :" << ::WSAGetLastError() << std::endl;
		socket = INVALID_SOCKET;
	} else  { // ok so connect
		int iResult = ::connect( socket, (PSOCKADDR)&name, sizeof(name));
		if (iResult == SOCKET_ERROR) {
			
			int err = ::WSAGetLastError();
			
			char* reason = "";
			switch (err) {
			case WSAECONNREFUSED: reason = "Connection Refused"; break;
			case WSAENETUNREACH: reason = "Host Unreachable"; break;
			case WSAETIMEDOUT: reason = "Timed out"; break;
			}

			std::cerr << "connect() failed with error code :" << err << " " << reason << std::endl;
			::closesocket(socket);
			socket = INVALID_SOCKET;
		}
	}

	return socket != INVALID_SOCKET;
}


TCPClient::TCPClient(const char* host, int port)
	: socket(INVALID_SOCKET)
{
	int iResult = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
    } else {
	   //setup address structure
		memset((char *) &name, 0, sizeof(name));
		name.sin_family = AF_INET;
		name.sin_port = ::htons(port);  // convert to network byte order
		//name.sin_addr.S_un.S_addr = ::inet_addr(host);
		::inet_pton(AF_INET, host, &(name.sin_addr)); // presentation to network

		// and make the connection.
		connect();
	}

     
 
}


TCPClient::~TCPClient(void) {	
	if(socket != INVALID_SOCKET) {
		::shutdown(socket, SD_SEND);
		::closesocket(socket);
	}

    ::WSACleanup();
}

// Attempts to send a message.  Returns the error code, 0 if successfull.
// closes the socket if send fails.
int TCPClient::trySend(const std::string& msg) {
	assert(socket != INVALID_SOCKET);
	int err = 0;
	if (::send(socket, msg.data(), msg.length() , 0) == SOCKET_ERROR)   {
		err = ::WSAGetLastError();
		std::cerr << "send() failed with error code : " << err << std::endl;

		// Terminate connection
		::shutdown(socket, SD_SEND);
		::closesocket(socket);
		socket = INVALID_SOCKET;
	}
	return err;
}

void TCPClient::send(const std::string& msg){
	if(socket != INVALID_SOCKET) {
		int err = trySend(msg);

		// Provide limited retry.
		if(err == WSAECONNABORTED || err == WSAECONNRESET) {
			std::cerr << "Reconnecting..." << std::endl;
			connect();
			trySend(msg);
		}
	}
}