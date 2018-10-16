/**
 * 
 */
package uk.co.alvagem.condorjfx.controller;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.UnknownHostException;

/**
 * Class to send strings as IP datagrams to a given host & port.
 * @author bruce.porteous
 *
 */
public class DatagramLink {

	public final static int DEFAULT_PORT = 55278;
	public final static String DEFAULT_ADDRESS = "127.0.0.1";
	
	private InetAddress address;
	private int port;
	
	/**
	 * @throws UnknownHostException 
	 * 
	 */
	public DatagramLink() throws UnknownHostException {
		this(DEFAULT_ADDRESS, DEFAULT_PORT );
	}
	
	public DatagramLink(String host) throws UnknownHostException {
		this(host, DEFAULT_PORT);
	}
	
	public DatagramLink(String host, int port) throws UnknownHostException {
		address = InetAddress.getByName(host);
		this.port = port;
	}

	/**
	 * @param message is the message to be sent.
	 * @throws UnsupportedEncodingException
	 * @throws IOException
	 * @throws SocketException
	 */
	public  void send(String message)
			throws UnsupportedEncodingException, 
			IOException, SocketException {
		
		byte[] data = message.getBytes("ASCII");
		
		try (DatagramSocket socket = new DatagramSocket();) {
			DatagramPacket packet = new DatagramPacket(data, data.length, address, port);
			socket.send(packet);
		}
	}

}
