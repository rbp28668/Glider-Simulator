#include "stdafx.h"
#include "ElectronicVario.h"


ElectronicVario::ElectronicVario()
	: then(0)
	, previous(0)
	, acc(0)
	, damping(1)
	, teFactor(0.2)
{
}


ElectronicVario::~ElectronicVario()
{
}

double ElectronicVario::update(double height, double tas, DWORD now)
{
	double energy = height + teFactor * 0.5 * tas * tas;  // Potential + kinetic. Assume mass = 1.  teFactor is a bit of a fudge as over compensates.
	double de = energy - previous;
	if (now > then) {
		double dt = (now - then) / 1000.0; // in seconds;
		double rate = de / dt;
		double df = damping / dt; // scale damping by time interval - the shorter the time interval the more damping as the more updates 
		acc = ((acc * df + rate) / (df + 1));
	}
	then = now;
	previous = energy;
	return acc;
}
