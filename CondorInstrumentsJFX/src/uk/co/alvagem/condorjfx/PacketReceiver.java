/**
 * 
 */
package uk.co.alvagem.condorjfx;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.SocketException;

import javafx.application.Platform;

/**
 * @author bruce.porteous
 *
 */
public class PacketReceiver implements Runnable{

	private Instruments instruments;
	private Commands commands;
	
	static final int BUFFER_SIZE = 1024;
    private DatagramSocket receiveSocket;
    byte[] receiveData;

	public PacketReceiver( Commands commands, int port) throws SocketException {
		this.instruments = null;
		this.commands = commands;
		receiveSocket = new DatagramSocket(port);
		receiveData = new byte[BUFFER_SIZE];
	}

 
	/* (non-Javadoc)
	 * @see java.lang.Runnable#run()
	 */
	@Override
	public void run() {

		boolean stop = false;
		while(!stop) {
			try {
				Packet packet = receive();
				Platform.runLater( new Updater(packet)); // process on JFX UI thread.
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	/*
	 * Runnable so we can process the received data on the JFX UI thread.
	 * This allows the code to update the UI without threading issues.
	 */
	private class Updater implements Runnable {

		Packet packet;
		
		Updater(Packet packet){
			this.packet = packet;
		}
		
		/* (non-Javadoc)
		 * @see java.lang.Runnable#run()
		 */
		@Override
		public void run() {
			if(instruments != null) {
				instruments.dispatch(packet);
			}
			commands.dispatch(packet);
		}
	}

	private Packet receive() throws IOException{
    
		receiveData = new byte[BUFFER_SIZE];
          DatagramPacket receivePacket = new DatagramPacket(receiveData, receiveData.length);
          receiveSocket.receive(receivePacket);
          
          Packet packet = new Packet(receivePacket.getData());
          return packet;
	}


	/**
	 * @param instruments
	 */
	public void setInstruments(Instruments instruments) {
		this.instruments = instruments;
	}
}
