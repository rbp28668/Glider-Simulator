/**
 * 
 */
package uk.co.alvagem.condorjfx.controller;

import java.util.HashMap;
import java.util.Map;

/**
 * Simple parser for command line options.
 * Expects options of the form --option value
 * Anything else is put together in a tail string.
 * @author bruce.porteous
 *
 */
class Options {

	private final static String TAIL = ""; // key can never be an option
	
	private Map<String,String> options = new HashMap<String,String>();
	
	private boolean hasError = false;
	
	public Options(String[] args) {
		parseOptions(args);
	}
	
	/**
	 * @param args
	 * @param message
	 * @return
	 */
	private Map<String, String> parseOptions(String[] args) {
		
		StringBuffer message = new StringBuffer();
		String arg = null;
		try {
			for(int idx = 0; idx < args.length; ++idx) {
				arg = args[idx];
				if(arg.startsWith("--")) {
					arg = arg.substring(2);
					++idx;
					options.put(arg, args[idx]);
				} else { // not a -- option  so message
					if(message.length() > 0) {
						message.append(' ');
					}
					message.append(arg);
				}
			}
			if(message.length() > 0) {
				options.put(TAIL, message.toString());
			}
		} catch (ArrayIndexOutOfBoundsException e) {
			hasError = true;
			System.err.println("Missing value for " + arg + " option");
		}
		return options;
	}
	
	public boolean hasError() {
		return hasError;
	}
	
	public String getTail() {
		return options.get(TAIL);
	}
	
	public String getString(String key, String defaultValue) {
		return options.getOrDefault(key, defaultValue);
	}
	
	public int getInt(String key, int defaultValue){
		String val = options.get(key);
		return (val != null) ? Integer.parseInt(val) : defaultValue;
	}

	public float getFloat(String key, float defaultValue){
		String val = options.get(key);
		return (val != null) ? Float.parseFloat(val) : defaultValue;
	}

	public double getDouble(String key, double defaultValue){
		String val = options.get(key);
		return (val != null) ? Double.parseDouble(val) : defaultValue;
	}

	public boolean getBoolean(String key, boolean defaultValue){
		String val = options.get(key);
		return (val != null) ? Boolean.parseBoolean(val) : defaultValue;
	}
}