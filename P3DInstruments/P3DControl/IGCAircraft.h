#pragma once
#include "Trace.h"
#include "..//P3DCommon/ExternalSimVehicle.h"

class Prepar3D;
class IGCFile;

class IGCAircraft
{
protected:
	Trace trace;
	double baseAltitude; // of trace
	double getBaseAltitude(IGCFile* igc);
	double initialGroundHeight;
	bool initialGroundHeightSet;

public:
	IGCAircraft(IGCFile* igc);
	void simulate(const SimInputData& inputData, SimOutputData& outputData, double t);
	SIMCONNECT_DATA_INITPOSITION initialPosition();

};


class UserIGCAircraft : public ExternalSimVehicle, public IGCAircraft {

public:
	UserIGCAircraft(Prepar3D* p3d, IGCFile* igc, const char* containerName);
	virtual void simulate(const SimInputData& inputData, SimOutputData& outputData, double t);
	virtual ~UserIGCAircraft();
};

class AIIGCAircraft : public AIExternalSimVehicle, public IGCAircraft {

public:
	AIIGCAircraft(Prepar3D* p3d, IGCFile* igc, const char* containerName);
	virtual void simulate(const SimInputData& inputData, SimOutputData& outputData, double t);
	virtual SIMCONNECT_DATA_INITPOSITION initialPosition();
	virtual ~AIIGCAircraft();

};
