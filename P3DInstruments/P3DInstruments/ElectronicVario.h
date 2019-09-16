#pragma once
class ElectronicVario
{
	DWORD then;
	double previous;
	double damping;
	double acc;
	double teFactor;

public:
	ElectronicVario();
	~ElectronicVario();

	void setDamping(double damping) { this->damping = damping; }

	double update(double height, double tas, DWORD now);


};

