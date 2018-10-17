#include "StdAfx.h"
#include <math.h>
#include "MechanicalVario.h"

double MechanicalVario::CAPACITY = 450.0; //ml
double MechanicalVario::RHO0 = 1.225; // Sea level density: kg/m3	
double MechanicalVario::P0 = 1013.25; // Sea level pressure: hPa
double MechanicalVario::T0 = 288.15; // Sea level temperature: K (15C)
double MechanicalVario::R = 287.04; // real gas constant for air

MechanicalVario::MechanicalVario(void)
	: tLast(0)
	, pcLast(0)
	, lastResult(0)
{
}


MechanicalVario::~MechanicalVario(void)
{
}


	/**
	 * Calculates the pressure at a given height in metres.
	 * @param height
	 * @return pressure in Pa.
	 */
	double MechanicalVario::pressure(double height) {
		return P0 * 100.0 * pow(1.0 - 0.0065 * height / T0, 5.2561);
	}
	
	/**
	 * Calculates the temperature in Kelvin at a given height in metres.
	 * @param height
	 * @return the temperature in Kelvin.
	 */
	double MechanicalVario::temp(double height) {
		return T0 - 0.0065 * height;
	}
	
	 double MechanicalVario::density(double height) {
		return pressure(height) / ( R * temp(height)); // perfect gas equation.
	}
	
	double MechanicalVario::density(double pressure, double tempK) {
		return pressure / (R * tempK);
	}
	
	/**
	 * This one measures the difference in pressure between static pressure fed into the instrument
	 * case and the capacity bottle fed into the aneroid capsule.  The capilliary joins the capacity to
	 * the instrument case. The static pressure into the instrument case comes from the TE probe.
	 * Pressure on TE probe: Pt = Ps - 1/2 rho V^2
	 * Where Ps is staic pressure rho is air density and V is speed.
	 */
	double MechanicalVario::update(double height, double tas, DWORD now) {
	
		double Ps = pressure(height);
		double T = temp(height);
		double rho = density(Ps,T);
		
		// Pressure at TE probe:  static pressure - airspeed component.
		double Pte = Ps - 0.5 * rho * tas * tas;


		double ts;		// time since last reading.
		double Pc;  // capacity pressure.

		// Just in case time hasn't moved on enough to register - windows
		// tick is fairly coarse grained.
		if(now == tLast) {
			return lastResult;
		}

		if(tLast == 0) { // then first time through;
			Pc = Pte;
			pcLast = Pte;
			ts = 1;
		} else {
			ts =  ((double)(now - tLast)) / 1000.0; // in seconds.

			// Assumptions - flow through capilliary is laminar and hence volumetric flow rate is
			// proportional to difference in pressure.  We're ignoring changes in
			// viscosity due to temperature.
			// See  Hagen–Poiseuille equation - assuming all constants folded down to one
			// scale factor "capilliary".
			
			double capilliary = 0.005;
			double flowIn = ts * (Pte - pcLast) * capilliary;
			double densityTE = density(Pte, T);
			double massIn = flowIn * densityTE;
			
			// Note use of T below - using nominal external value, not temp in capacity.
			// Assumption is that this is close enough as capacity pressure usually close
			// to ambient. 
			double densityCap = density(pcLast,T);
			double massCap = CAPACITY * densityCap + massIn;
			densityCap = massCap / CAPACITY;
			Pc = densityCap * (R * T);
		}
		
		double dP = Pc - pcLast; // difference in pressure 
		// ok, so 1 hPa = 30 feet at sea level or 1Pa = 0.3 feet.
		double dm = dP * 0.3 * 0.3048; // nominal height change in m based on pressure.
		
		tLast = now;
		pcLast = Pc;
		
		lastResult = -dm / ts; // dist and time to vertical speed. -ve as increasing pressure is down.
		return lastResult;
	}
