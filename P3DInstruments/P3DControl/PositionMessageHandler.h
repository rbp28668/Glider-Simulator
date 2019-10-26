#pragma once
#include "MessageHandler.h"
#include "Simulator.h"
#include "SimState.h"

class JSONWriter;
class File;

class PositionMessageHandler :
	public MessageHandler
{
	Simulator* pSim;
	void write(const SimState::Data& data, JSONWriter& json);
	void getPositionMetadata(File& f, std::string& title, std::string& description);
public:
	PositionMessageHandler(Prepar3D* p3d);
	virtual ~PositionMessageHandler();
	virtual void run(const std::string& cmd, const APIParameters& params, std::string& output);

	void show(std::string& output);
	void available(std::string& output);
	void set(int count, std::string& output);
	void back(int count, std::string& output);
	void load(const std::string& file, std::string& output);
	void save(const std::string& file, const std::string& title, const std::string& description, std::string& output);
	void list(const std::string& filter, std::string& output);
	void up(int feet, std::string& output);
	void down(int feet, std::string& output);
	void clearHistory(std::string& output);

};

