#pragma once
#include "../P3DCommon/UDPClient.h"

class CondorUDP : public UDPClient
{

public:
	// Instrument data
	double time; // =12.2146622328029, decimal hours
	double airspeed; //=24.96994972229, m-s
	double altitude; //=4477.5869140625, feet (? doc says m)
	double vario; // =-3.08353328704834, m/s
	double compass; // =82.3176193237305, degrees
	double slipball; // =-8.19163033156656E-5, radians
	double turnrate; // =0.0453215204179287, radians per sec
	double yawstringangle; // =0, radians
	char radiofrequency[8]; // =123.5, MHz
	double yaw; // =1.43671345710754, radians
	double pitch; // =1.4968318939209, radians
	double bank; // =0.0418588891625404, radians
	//quaternionx=0.308790266513824
	//quaterniony=0.654939413070679
	//quaternionz=0.319486290216446
	//quaternionw=0.610836923122406
	//ax=-6.93911218643188
	//ay=9.15493011474609
	//az=-10.3173704147339
	//vx=-3.26996803283691
	//vy=2.55103325843811
	//vz=27.0995941162109
	//rollrate=-0.00251091504469514
	//pitchrate=0.406363248825073
	//yawrate=0.0232510436326265
	double gforce; // =1.17204143123646

	CondorUDP(const char* host, int port);
	virtual ~CondorUDP(void);

	void send();
};

