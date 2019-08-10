#include "StdAfx.h"
#include <iostream>
#include <Ws2tcpip.h>
#include "UDPClient.h"

#pragma comment(lib, "Ws2_32.lib")


UDPClient::UDPClient(const char* host, int port)
{
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
}

void UDPClient::send(const std::string& msg) {
		if (::sendto(socketHandle, msg.data(), (int)msg.length() , 0 , (struct sockaddr *) &si, slen) == SOCKET_ERROR)   {
        std::cerr << "sendto() failed with error code : " << ::WSAGetLastError() << std::endl;
    }
}

