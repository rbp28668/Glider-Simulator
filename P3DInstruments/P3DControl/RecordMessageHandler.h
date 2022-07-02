#pragma once
#include "MessageHandler.h"
#include "Folder.h"

class Simulator;

class RecordMessageHandler :
	public MessageHandler
{

public:
	RecordMessageHandler(Prepar3D* p3d);
	virtual void run(const std::string& cmd, const APIParameters& params, std::string& output);

	static bool start(Simulator* pSim);
	static bool stop(Simulator* pSim, const std::string& title, const std::string& description);
	static bool playback(Simulator* pSim, const std::string& name);
	static bool analyse(Simulator* pSim);
	static File::ListT& list(File::ListT& files);

private:
	void startRecording(std::string& output);
	void stopRecordingAndSave(const std::string& title, const std::string& description, std::string& output);
	void playbackRecording(const std::string& name, std::string& output);
	void saveAnalysis(std::string& output);
	void listRecordings(std::string& output);
};

