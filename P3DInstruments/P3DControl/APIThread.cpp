#include "stdafx.h"
#include "APIThread.h"
#include "RESTAPI.h"

APIThread::APIThread(CommandInterpreter* pInterpreter) 
	: Thread(false)
	, pInterpreter(pInterpreter)
{
}

APIThread::~APIThread()
{
}

unsigned APIThread::run()
{
	RESTAPI api(pInterpreter);
	api.receiveRequests();
	return 0;
}

void APIThread::stop() {
	// TODO, figure out how to cleanly terminate the thread.
}
