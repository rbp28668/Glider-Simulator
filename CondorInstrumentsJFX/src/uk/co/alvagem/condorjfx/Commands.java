/**
 * 
 */
package uk.co.alvagem.condorjfx;

import java.util.HashMap;
import java.util.Map;

/**
 * @author bruce.porteous
 *
 */
public class Commands {

	private Map<String, Command> commands = new HashMap<String, Command>();
	
	/**
	 * 
	 */
	public Commands() {
	}

	public void add(Command command) {
		commands.put(command.getKey(), command);
	}
	public void dispatch(Packet packet) {
		
		for(String cmd : packet.getCommands()) {
			
			String parts[] = cmd.split("\\b");
			Command command = commands.get(parts[0]);
			if(command != null) {
				command.execute(cmd);
			} else {
				System.err.println("Unrecognised command " + cmd);
			}
		}
	}

}
