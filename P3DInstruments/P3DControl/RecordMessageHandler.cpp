#include "stdafx.h"
#include "RecordMessageHandler.h"

RecordMessageHandler::RecordMessageHandler(Prepar3D* p3d)
	: MessageHandler(p3d, "record")
{
}

void RecordMessageHandler::run(const std::string& cmd, const APIParameters& params, std::string& output)
{
}
