package uk.co.alvagem.condorjfx.control;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.util.Map;

import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.geometry.Insets;
import javafx.scene.Node;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.control.Button;
import javafx.scene.layout.Background;
import javafx.scene.layout.BackgroundFill;
import javafx.scene.layout.Border;
import javafx.scene.layout.CornerRadii;
import javafx.scene.layout.GridPane;
import javafx.scene.paint.Color;
import javafx.scene.text.Font;
import uk.co.alvagem.condorjfx.Instrument;

public class SlewControl implements Instrument {

	private String host;
	private InetAddress address;
	private int port;
	private double fontSize;

	
	public SlewControl(String host, int port) throws Exception {
		this(host, port, 20);
	}

	public SlewControl(String host, int port, double fontSize) throws Exception {
		this.host = host;
		this.port = port;
		this.fontSize = fontSize;
		address = InetAddress.getByName(host);
	}

	
	
	public String getHost() {
		return host;
	}


	public int getPort() {
		return port;
	}

	public double getFontSize() {
		return fontSize;
	}

	@Override
	public void process(Map<String, String> readings) {
	}

	@Override
	public void drawDiagnostic(GraphicsContext gc) {
	}

	@Override
	public void updated() {
	}

	@Override
	public Node getInstrumentNode() throws Exception {
		GridPane grid = new GridPane();
		grid.setHgap(10);
		grid.setVgap(10);
		grid.setPadding(new Insets(0, 10, 0, 10));
		Background background = new Background(
				new BackgroundFill(Color.rgb(60,60,60), 
				new CornerRadii(10), 
				new Insets(-10,-10,-10,-10)));
		grid.setBackground(background);

		Font font = Font.font("Sans Serif", fontSize);
		
		// Altitude
		add(grid,font,"Up Fast","SLEW_ALTIT_UP_FAST",0,0);
		add(grid,font,"Up","SLEW_ALTIT_UP_SLOW",0,1);
		add(grid,font,"Stop","SLEW_ALTIT_FREEZE",0,2);
		add(grid,font,"Down","SLEW_ALTIT_DN_SLOW",0,3);
		add(grid,font,"Down Fast","SLEW_ALTIT_DN_FAST",0,4);

		// Translation
		add(grid,font,"Ahead","SLEW_AHEAD_PLUS",2,1);	
		add(grid,font,"Left","SLEW_LEFT",1,2);
		add(grid,font,"Stop","SLEW_FREEZE",2,2);
		add(grid,font,"Right","SLEW_RIGHT",3,2);
		add(grid,font,"Back","SLEW_AHEAD_MINUS",2,3);

		// Pitch/roll/yaw
		add(grid,font,"Pitch Up Fast","SLEW_PITCH_UP_FAST",2,5);
		add(grid,font,"Pitch Up","SLEW_PITCH_UP_SLOW",2,6);	
		add(grid,font,"Pitch Up Faster","SLEW_PITCH_PLUS",2,7);	
		add(grid,font,"Bank Left","SLEW_BANK_MINUS",1,8);	 
		add(grid,font,"Stop","SLEW_FREEZE",2,8);	
		add(grid,font,"Bank Right","SLEW_BANK_PLUS",3,8);
		add(grid,font,"Pitch Down Faster","SLEW_PITCH_MINUS",2,9);	
		add(grid,font,"Pitch Down","SLEW_PITCH_DN_SLOW",2,10);	
		add(grid,font,"Pitch Down Fast","SLEW_PITCH_DN_FAST",2,11);	
		add(grid,font,"Yaw Left","SLEW_HEADING_MINUS",1,12);
		add(grid,font,"Stop","SLEW_FREEZE",2,12);
		add(grid,font,"Yaw Right","SLEW_HEADING_PLUS",3,12);

		// Control
		add(grid,font,"Slew ON","SLEW_ON",0,9);
		add(grid,font,"Slew Off","SLEW_OFF",0,10);
		add(grid,font,"Slew Reset","SLEW_RESET",0,11);

		Button close = new Button("Close");
		close.setFont(font);
		grid.add(close, 0, 12);

		return grid;
	}

	private void add(GridPane grid,Font font, String text, String cmd, int columnIndex, int rowIndex) throws Exception {
		CommandButton btn = new CommandButton(text, cmd);
		btn.setFont(font);
		grid.add(btn, columnIndex, rowIndex);
	}

	@Override
	public String[] getConfigFields() {
		String[] fields = {"host","port"};
		return fields;
	}

	class CommandButton extends Button {
		private byte[] command;

		CommandButton(String text, String cmd) throws Exception {
			super(text);
			this.command = ("cmd:" + cmd).getBytes("UTF-8");

			setOnAction(new EventHandler<ActionEvent>() {
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

		}

	}
}
