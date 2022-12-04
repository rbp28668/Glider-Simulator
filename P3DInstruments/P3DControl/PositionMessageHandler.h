#pragma once
#include "MessageHandler.h"
#include "Simulator.h"
#include "SimState.h"
#include "Folder.h"

class JSONWriter;
class File;

class PositionMessageHandler :
	public MessageHandler
{
	Simulator* pSim;
	static void write(const SimState::Data& data, JSONWriter& json);
	static void freeze(Simulator* pSim);
	static void unfreeze(Simulator* pSim);

public:
	PositionMessageHandler(Prepar3D* p3d);
	virtual ~PositionMessageHandler();
	virtual void run(const std::string& cmd, const APIParameters& params, std::string& output);

	static void getPositionMetadata(Simulator* pSim, File& f, std::string& title, std::string& description);

	static SimState::Data show(Simulator* pSim);
	static int available(Simulator* pSim);
	static int start(Simulator* pSim);
	static void stop(Simulator* pSim);

	static void set(Simulator* pSim, int count);
	static void back(Simulator* pSim, int count);
	static bool load(Simulator* pSim, const std::string& file, std::string& err);
	static void save(Simulator* pSim, const std::string& file, const std::string& title, const std::string& description);
	static void list(Simulator* pSim, const std::string& filter, File::ListT& files);
	static void up(Simulator* pSim, int feet);
	static void down(Simulator* pSim, int feet);
	static void clearHistory(Simulator* pSim);

private:
	void show(std::string& output);
	void available(std::string& output);
	void start(std::string& output);
	void stop(std::string& output);
	void set(int count, std::string& output);
	void back(int count, std::string& output);
	void load(const std::string& file, std::string& output);
	void save(const std::string& file, const std::string& title, const std::string& description, std::string& output);
	void list(const std::string& filter, std::string& output);
	void up(int feet, std::string& output);
	void down(int feet, std::string& output);
	void clearHistory(std::string& output);

};

