#pragma once
#include "MessageHandler.h"

class RecordMessageHandler :
	public MessageHandler
{

public:
	RecordMessageHandler(Prepar3D* p3d);
	virtual void run(const std::string& cmd, const APIParameters& params, std::string& output);
	void startRecording(std::string& output);
	void stopRecordingAndSave(const std::string& title, const std::string& description, std::string& output);
	void playbackRecording(const std::string& name, std::string& output);
	void saveAnalysis(std::string& output);
	void listRecordings(std::string& output);
};

