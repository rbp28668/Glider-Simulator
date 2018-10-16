/**
 * 
 */
package uk.co.alvagem.condorjfx.controller;


/**
 * @author bruce.porteous
 *
 */
public class Send {

	/**
	 * 
	 */
	public Send() {
	}

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		
		try {
			Options options = new Options(args);
			
			String host = options.getString("host", DatagramLink.DEFAULT_ADDRESS);
			int port = options.getInt("port", DatagramLink.DEFAULT_PORT);
			
			DatagramLink link = new DatagramLink(host,port);
			
			link.send(options.getTail());
			
		} catch (Exception e) {
			System.err.println("Unable to send message due to " + e.getMessage());
		}	
	}
}
