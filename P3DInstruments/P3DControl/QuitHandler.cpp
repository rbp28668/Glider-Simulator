#include "stdafx.h"
#include <assert.h>
#include "QuitHandler.h"
#include "MessageThread.h"

QuitHandler::QuitHandler(MessageThread* pmt)
	: name("quit")
{
	assert(pmt != 0);
	this->pmt = pmt;
}

const std::string& QuitHandler::getName() {
	return name;
}

void QuitHandler::run(const std::string& params) {
	pmt->stop();
}
