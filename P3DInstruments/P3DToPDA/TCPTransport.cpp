#include "StdAfx.h"
#include "TCPTransport.h"


TCPTransport::TCPTransport(const char* host, int port)
	:tcp(host, port)
	, lastSent(0)
	, delay(500) // default send every 1/2 second
{
}


TCPTransport::~TCPTransport(void)
{
}


bool TCPTransport::canSend() {
	if( (::GetTickCount() - lastSent) > delay ) {

		// If connection never made or dropped, try to reconnect.
		if(!tcp.isConnected()) {
			if(!tcp.reconnect()) {
				lastSent = ::GetTickCount(); // make sure we delay connection attempts.
			}
		}

		return tcp.isConnected();
	}
	return false;
}

void TCPTransport::send(std::string& str) {
	tcp.send(str);
	lastSent = ::GetTickCount();
}

