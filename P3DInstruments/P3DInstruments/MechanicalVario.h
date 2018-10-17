#pragma once
class MechanicalVario
{

	static double CAPACITY; //ml
	static double RHO0; // Sea level density: kg/m3	
	static double P0; // Sea level pressure: hPa
	static double T0; // Sea level temperature: K (15C)
	static double R; // real gas constant for air
	
	DWORD tLast;  // last tick this was calculated.
	double pcLast; // previous capacity pressure
	double lastResult; // result of previous pass.

	double pressure(double height);
	double temp(double height);
	double density(double height);
	double density(double pressure, double tempK);


public:
	double update(double height, double tas, DWORD now);

	MechanicalVario(void);
	virtual ~MechanicalVario(void);
};

