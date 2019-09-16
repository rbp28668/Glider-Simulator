#include "stdafx.h"
#include <assert.h>
#include<iostream>
#include "EventMessageHandler.h"




EventMessageHandler::EventMessageHandler(Prepar3D * p3d)
	: name("cmd")
	, commands(p3d)
{
	assert(p3d != 0);
	this->p3d = p3d;
}

EventMessageHandler::~EventMessageHandler()
{
}

const std::string & EventMessageHandler::getName()
{
	return name;
}

void EventMessageHandler::run(const std::string & params)
{
	if (commands.dispatchEvent(params)) {
		std::cout << "Unable to run P3D command " << params.c_str() << std::endl;
	}
}
