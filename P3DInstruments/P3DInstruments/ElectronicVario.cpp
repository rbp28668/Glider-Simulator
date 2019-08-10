#include "stdafx.h"
#include "ElectronicVario.h"


ElectronicVario::ElectronicVario()
	: then(0)
	, previous(0)
	, acc(0)
	, damping(1000)
{
}


ElectronicVario::~ElectronicVario()
{
}

double ElectronicVario::update(double height, double tas, DWORD now)
{
	double energy = height + 0.5 * tas * tas;  // Potential + kinetic. Assume mass = 1
	double de = energy - previous;
	if (now > then) {
		double dt = (now - then) / 1000.0; // in seconds;
		double rate = de / dt;
		double df = damping / dt; // scale damping by time interval - the shorter the time interval the more damping as the more updates 
		acc = ((acc * df + rate) / (damping + 1));
	}
	then = now;
	previous = energy;
	return acc;
}
