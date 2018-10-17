#include "StdAfx.h"
#include <iostream>
#include <Ws2tcpip.h>
#include "UDPClient.h"


UDPClient::UDPClient(const char* host, int port)
{
	int iResult = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        //exit(1);
    }

	   //create socket
    if ( (socketHandle = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)  {
        std::cerr << "socket() failed with error code :" << ::WSAGetLastError() << std::endl;
        //exit(1);
    }
     
    //setup address structure
	slen=sizeof(si);
    memset((char *) &si, 0, slen);
    si.sin_family = AF_INET;
    si.sin_port = ::htons(port);  // convert to network byte order
	::inet_pton(AF_INET, host, &(si.sin_addr)); // presentation to network

}


UDPClient::~UDPClient(void)
{
    ::closesocket(socketHandle);
    ::WSACleanup();
}

void UDPClient::send(const std::string& msg) {
		if (::sendto(socketHandle, msg.data(), msg.length() , 0 , (struct sockaddr *) &si, slen) == SOCKET_ERROR)   {
        std::cerr << "sendto() failed with error code : " << ::WSAGetLastError() << std::endl;
    }
}

