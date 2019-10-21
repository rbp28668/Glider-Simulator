#pragma once
#include <fstream>
#include "..//P3DCommon/SimObjectData.h"
#include "Folder.h"
#include "IGCFile.h"

class IGCFlightRecorder
{
	struct Data {
		double latitude;
		double longitude;
		double altitude;
		double gpsLatitude;
		double gpsLongitude;
		double gpsAltitude;
		double zuluTime;
		double zuluDay;
		double zuluMonth;
		double zuluYear;
	};

	static SimObjectData::DataItem dataItems[];

	class FRData : public SimObjectData {
		IGCFlightRecorder* parent;
	public:
		FRData(IGCFlightRecorder* parent, Prepar3D* pTargetSim, SimObjectData::DataItem* pItems);
		virtual void onData(void* pData, SimObject* pObject);
		virtual ~FRData();
	};

	Prepar3D* p3d;
	int interval; // per sample
	FRData frData; // Data format from sim
	SimObjectDataRequest* pRequest; // to request the data
	IGCFile igc; // to write out the data
	std::ofstream output; // to this stream

	File createFileName(Directory& folder);
public:

	void setInterval(int seconds) { interval = seconds; }
	void setP1(const std::string& p1) { igc.setP1(p1); }
	void setP2(const std::string& p2) { igc.setP2(p2); }
	void setGliderType(const std::string& gliderType) { igc.setGliderType(gliderType); }
	void setRegistration(const std::string& registration) { igc.setRegistration(registration); }
	void setCompetitionClass(const std::string& competitionClass) { igc.setCompetitionClass(competitionClass); }
	void setCompetitionId(const std::string& competitionId) { igc.setCompetitionId(competitionId); }

	IGCFlightRecorder(Prepar3D* pTargetSim);
	~IGCFlightRecorder();

	void onData(Data* pData);

	void start(Directory& directory);
	void stop();
	bool running();

};

