package uk.co.alvagem.condorjfx.control;

import java.util.Map;

import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.scene.Node;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.control.Button;
import javafx.scene.text.Font;
import uk.co.alvagem.condorjfx.Instrument;
import uk.co.alvagem.condorjfx.Main;

public class SelectPanelButton implements Instrument {

	private Button button;
	private String text;
	private String panelName;
	private double fontSize;
	
	public SelectPanelButton( String text, String panelName) throws Exception{
		this(text, panelName, 20);
	}

	public SelectPanelButton( String text, String panelName, double fontSize) throws Exception{
		this.text = text;
		this.panelName = panelName;
		this.fontSize = fontSize;
	}

	
	
	public String getText() {
		return text;
	}

	public String getPanelName() {
		return panelName;
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
	        	 Main.getApp().selectPanel(panelName);
		    }
		});
		
		return button;
	}

	@Override
	public String[] getConfigFields() {
		String[] fields = {"text","panelName"};
		return fields;
	}


	@Override
	public void drawDiagnostic(GraphicsContext gc) {
	}

	
}
