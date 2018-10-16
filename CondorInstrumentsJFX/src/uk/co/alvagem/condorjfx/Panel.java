/**
 * 
 */
package uk.co.alvagem.condorjfx;


/**
 * @author bruce.porteous
 *
 */
public class Panel {
	private String name;
	private Instruments instruments = new Instruments();
	/**
	 * @return the name
	 */
	public String getName() {
		return name;
	}
	/**
	 * @param name the name to set
	 */
	public void setName(String name) {
		this.name = name;
	}
	/**
	 * @return the instruments
	 */
	public Instruments getInstruments() {
		return instruments;
	}
	

}
