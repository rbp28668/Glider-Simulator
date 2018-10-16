/**
 * 
 */
package uk.co.alvagem.condorjfx;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

// data packet is an ASCII stream of ‘parameter=value’ pairs
/**
 * Holds a parsed data packet for distribution to instruments & command handlers.
 * @author bruce.porteous
 *
 */
class Packet {
	private List<String> commands = new LinkedList<String>();
	private Map<String,String> readings = new HashMap<String,String>();

	Packet(byte[] data) throws IOException{
		InputStream is = new BufferedInputStream(new ByteArrayInputStream(data));
		BufferedReader in = new BufferedReader(new InputStreamReader(is,"ASCII"));

		String line = in.readLine();
		while(line != null) {
			line = line.trim();
			String[] parts = line.split("=");
			if(parts.length == 2) {
				readings.put(parts[0], parts[1]);
			} else {
				if(!line.isEmpty()){
					commands.add(line);
				}
			}
			line = in.readLine();
		}
		in.close();

	}

	/**
	 * @return the commands
	 */
	public List<String> getCommands() {
		return commands;
	}

	/**
	 * @return the readings
	 */
	public Map<String, String> getReadings() {
		return readings;
	}


}