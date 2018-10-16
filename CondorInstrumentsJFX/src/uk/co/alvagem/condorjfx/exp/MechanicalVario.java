/**
 * 
 */
package uk.co.alvagem.condorjfx.exp;

/**
 * @author bruce.porteous
 *
 */
public class MechanicalVario {

	private final static double CAPACITY = 450.0; //ml
	private final static double RHO0 = 1.225; // Sea level density: kg/m3	
	private final static double P0 = 1013.25; // Sea level pressure: hPa
	private final static double T0 = 288.15; // Sea level temperature: K (15C)
	private final static double R = 287.04; // real gas constant for air
	private long tLast = 0;  // last tick this was calculated.
	private double pcLast = 0; // previous capacity pressure
	
	
	/**
	 * 
	 */
	public MechanicalVario() {
		// TODO Auto-generated constructor stub
	}

	
	/**
	 * Calculates the pressure at a given height in metres.
	 * @param height
	 * @return pressure in Pa.
	 */
	private double pressure(double height) {
		return P0 * 100.0 * Math.pow(1.0 - 0.0065 * height / T0, 5.2561);
	}
	
	/**
	 * Calculates the temperature in Kelvin at a given height in metres.
	 * @param height
	 * @return the temperature in Kelvin.
	 */
	private double temp(double height) {
		return T0 - 0.0065 * height;
	}
	
	private double density(double height) {
		return pressure(height) / ( R * temp(height)); // perfect gas equation.
	}
	
	private double density(double pressure, double tempK) {
		return pressure / (R * tempK);
	}
	
	/**
	 * This one measures the difference in pressure between static pressure fed into the instrument
	 * case and the capacity bottle fed into the aneroid capsule.  The capilliary joins the capacity to
	 * the instrument case. The static pressure into the instrument case comes from the TE probe.
	 * Pressure on TE probe: Pt = Ps - 1/2 rho V^2
	 * Where Ps is staic pressure rho is air density and V is speed.
	 */
	public double update(double height, double tas, long now) {
	
		double Ps = pressure(height);
		double T = temp(height);
		double rho = density(Ps,T);
		
		// Pressure at TE probe:  static pressure - airspeed component.
		double Pte = Ps - 0.5 * rho * tas * tas;


		double ts;		// time since last reading.
		double Pc;  // capacity pressure.

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
			
			final double capilliary = 0.005;
			final double flowIn = ts * (Pte - pcLast) * capilliary;
			final double densityTE = density(Pte, T);
			final double massIn = flowIn * densityTE;
			
			// Note use of T below - using nominal external value, not temp in capacity.
			// Assumption is that this is close enough as capacity pressure usually close
			// to ambient. 
			double densityCap = density(pcLast,T);
			final double massCap = CAPACITY * densityCap + massIn;
			densityCap = massCap / CAPACITY;
			Pc = densityCap * (R * T);
		}
		
		double dP = Pc - pcLast; // difference in pressure 
		// ok, so 1 hPa = 30 feet at sea level or 1Pa = 0.3 feet.
		double dm = dP * 0.3 * 0.3048; // nominal height change in m based on pressure.
		
		tLast = now;
		pcLast = Pc;
		
		return -dm / ts;  // dist and time to vertical speed. -ve as increasing pressure is down.
	}
	
	/**
	 * See http://www.clearnav.net/Technical/Variometer%20pneumatics.pdf
	 * Winter various measure flow to and from the capacity bottle.
	 */
	public void updateWinter() {
		
	}
	
	
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		
		MechanicalVario vario = new MechanicalVario();
		
		double climb = 0.0;
		double accel = 0;
		double height = 100;
		double tas = 25;
		final double dt = 0.1;
		final double G = 9.81;
		
		double hEquivLast = height;
		
		for(long tms=0; tms<10000; tms += 100) {
			
			double t = (double)tms / 1000.0;

			if(tms >= 1000) {
				climb = 1.0;
			}
			
			if(tms >= 5000) {
				accel = 2;
			}
			
			height += climb * dt;
			tas += accel * dt;
		
			// Work out the height equivalent to the total energy and 
			// convert to a rate of climb.
			double PE = G * height;
			double KE = 0.5 * tas * tas;
			double E = PE + KE;
			double hEquiv = E / G;
			double dh = hEquiv - hEquivLast;
			hEquivLast = hEquiv;
			double upEquiv = dh / dt;
			
			double up = vario.update(height, tas, tms);
			System.out.println(t + "\t" + climb +  "\t" + tas + "\t" + upEquiv + "\t" + up);
		}
	}
	

}
