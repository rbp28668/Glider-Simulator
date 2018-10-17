#include "StdAfx.h"
#include "UDPTransport.h"


UDPTransport::UDPTransport(const char* host, int port)
	:udp(host,port)
	, lastSent(0)
	, delay(500) // default send every 1/2 second
{
}


UDPTransport::~UDPTransport(void)
{
}

bool UDPTransport::canSend() {
	return (::GetTickCount() - lastSent) > delay;
}

void UDPTransport::send(std::string& str){
	udp.send(str);
	lastSent = ::GetTickCount();
}
