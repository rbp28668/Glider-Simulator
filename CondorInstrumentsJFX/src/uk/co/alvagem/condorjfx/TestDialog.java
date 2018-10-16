/**
 * 
 */
package uk.co.alvagem.condorjfx;


import java.io.File;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.LinkedHashMap;
import java.util.Map;

import uk.co.alvagem.condorjfx.config.Configurator;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.FlowPane;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.Pane;
import javafx.scene.paint.Color;
import javafx.stage.FileChooser;
import javafx.stage.Modality;
import javafx.stage.Stage;


/**
 * @author bruce.porteous
 *
 */
public class TestDialog extends Stage {

	/**
	 * 
	 */
	@SuppressWarnings("unused")
	private static final long serialVersionUID = 1L;
	
	private ValuesPanel values;
	private Panel panel;
	private int port;
	
	public TestDialog(Stage owner, int port, Panel panel) {
		this.panel = panel;
		this.port = port;
		
		initOwner(owner);
		initModality(Modality.NONE);

		setTitle("Control");

        BorderPane pane = new BorderPane();
        Scene scene = new Scene(pane, 250, 150, Color.WHITE);
        setScene(scene);

		values = new ValuesPanel();
		pane.setCenter(values);
		
		pane.setBottom(createButtons(owner));
		
		show();
	}
	
	private Pane createButtons(Stage owner){
		final FlowPane pane = new FlowPane();
		
		final Button sendButton = new Button("Send");
		sendButton.setOnAction( ae -> {values.sendReadings();});
		pane.getChildren().add(sendButton);

		final Button saveButton = new Button("Save");
		saveButton.setOnAction( ae -> {
            FileChooser fileChooser = new FileChooser();
            
            //Set extension filter
            FileChooser.ExtensionFilter extFilter = new FileChooser.ExtensionFilter("XML files (*.xml)", "*.txt");
            fileChooser.getExtensionFilters().add(extFilter);
            
            //Show save file dialog
            File file = fileChooser.showSaveDialog(owner);
            
            if(file != null){
				Configurator config = new Configurator();
				Panel[] panels = new Panel[1];
				panels[0] = panel;
				try {
					config.savePanels(file.getAbsolutePath(), panels);
				} catch (Exception e) {
					e.printStackTrace();
				}
            }

			
		});
		pane.getChildren().add(saveButton);
		
		final Button  p0Button = new Button("Panel 0");
		p0Button.setOnAction( ae -> {setPanel(0);});
		pane.getChildren().add(p0Button);

		final Button  p1Button = new Button("Panel 1");
		p1Button.setOnAction( ae -> {setPanel(1);});
		pane.getChildren().add(p1Button);

		return pane;

	}
	
	private void setPanel(int idx) {

		try {
			StringBuilder builder = new StringBuilder();
			builder.append("Panel ");
			builder.append(Integer.toString(idx));
			builder.append("\n");
			
			send(builder);
		} catch (Exception e) {
			System.err.println("Unable to set panel");
		}
	}

	/**
	 * @param builder
	 * @throws UnsupportedEncodingException
	 * @throws UnknownHostException
	 * @throws IOException
	 * @throws SocketException
	 */
	protected void send(StringBuilder builder)
			throws UnsupportedEncodingException, UnknownHostException,
			IOException, SocketException {
		
		//System.out.println(builder.toString());
		byte[] data = builder.toString().getBytes("ASCII");
		
		try (DatagramSocket socket = new DatagramSocket();) {
			InetAddress address = InetAddress.getByName("127.0.0.1");
			DatagramPacket packet = new DatagramPacket(data, data.length, address, port);
			socket.send(packet);
		}
	}
	
	class ValuesPanel extends GridPane {
		
		@SuppressWarnings("unused")
		private static final long serialVersionUID = 1L;
		
		private Map<String, TextField> fields = new LinkedHashMap<String, TextField>();
		
		ValuesPanel() {
			int row = 0;
			addField("airspeed","0.0",row++);
			addField("altitude","0.0",row++);
			addField("vario","0.0",row++);
			addField("evario","0.0",row++);
			addField("slipball","0.0",row++);
			addField("turnrate","0.0",row++);
			addField("g","1.0",row++);
			addField("compass","0",row++);
			addField("radiofrequency","131.275",row++);
			addField("pitch","0.0",row++);
			addField("bank","0.0",row++);
		}
		
		private void addField(String tag, String initialValue, int row){
			add(new Label(tag),0,row);
			TextField textField = new TextField(initialValue);
			add(textField,1,row);
			fields.put(tag,textField);
		}
		
		void sendReadings(){
			try {
				StringBuilder builder = new StringBuilder();
				for(Map.Entry<String, TextField> field : fields.entrySet()){
					builder.append(field.getKey());
					builder.append('=');
					builder.append(field.getValue().getText());
					builder.append("\n");
				}
				
				send(builder);
				
				
			} catch (Exception e) {
				e.printStackTrace();
			}
		}

	}

}
