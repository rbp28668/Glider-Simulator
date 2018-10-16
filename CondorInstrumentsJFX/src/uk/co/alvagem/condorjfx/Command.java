/**
 * 
 */
package uk.co.alvagem.condorjfx;

/**
 * @author bruce.porteous
 *
 */
public abstract class Command {

	private String key;
	/**
	 * 
	 */
	public Command(String key) {
		this.key = key;
	}

	public String getKey() {
		return key;
	}

	/**
	 * 
	 */
	public abstract void execute(String cmd);
	
	
}
