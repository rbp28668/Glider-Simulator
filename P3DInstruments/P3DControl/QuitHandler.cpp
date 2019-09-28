#include "stdafx.h"
#include <assert.h>
#include "QuitHandler.h"
#include "MessageThread.h"

QuitHandler::QuitHandler(MessageThread* pmt)
	: MessageHandler(0, "quit")
{
	assert(pmt != 0);
	this->pmt = pmt;
}


void QuitHandler::run(const std::string& cmd, const APIParameters& params, std::string& output) {
	pmt->stop();
}
