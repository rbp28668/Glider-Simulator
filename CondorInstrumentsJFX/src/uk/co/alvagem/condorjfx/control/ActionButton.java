package uk.co.alvagem.condorjfx.control;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.util.Map;

import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.scene.Node;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.control.Button;
import javafx.scene.text.Font;
import uk.co.alvagem.condorjfx.Instrument;

public class ActionButton implements Instrument {

	private Button button;
	private String text;
	private int port;
	private byte[] command;
	private String host;
    private InetAddress address;
    private double fontSize;
	
	public ActionButton( String text, String cmd, String host, int port) throws Exception{
		this(text,cmd,host,port,20);
	}
	
	public ActionButton( String text, String cmd, String host, int port, double fontSize) throws Exception{
		this.text = text;
		this.port = port;
		this.host = host;
		this.fontSize = fontSize;
		this.command = cmd.getBytes("UTF-8");
		address = InetAddress.getByName(host);
	}

	
	
	public String getText() {
		return text;
	}

	public int getPort() {
		return port;
	}

	
	public String getHost() {
		return host;
	}

	public String getCmd() {
		return new String(command);
	}

	public double getFontSize() {
		return fontSize;
	}


	@Override
	public void process(Map<String, String> readings) {
	}

	@Override
	public void updated() {
	}

	@Override
	public Node getInstrumentNode() {
		
		button = new Button(text);
		Font font = Font.font("Sans Serif",fontSize);
		button.setFont(font);
		
		button.setOnAction(new EventHandler<ActionEvent>() {
		    @Override public void handle(ActionEvent e) {
		        
		         try {
					DatagramSocket socket = new DatagramSocket();
					DatagramPacket packet 
					  = new DatagramPacket(command, command.length, address, port);
					socket.send(packet);
					socket.close();
				} catch (IOException ex) {
					System.err.println("Can't send command: " + ex.getMessage());
				}
		    }
		});
		
		return button;
	}

	@Override
	public String[] getConfigFields() {
		String[] fields = {"text","cmd", "host","port", "fontSize"};
		return fields;
	}


	@Override
	public void drawDiagnostic(GraphicsContext gc) {
	}

	
}
