/**
 * 
 */
package uk.co.alvagem.condorjfx;

import java.util.Map;

import javafx.scene.Node;
import javafx.scene.canvas.GraphicsContext;

/**
 * @author bruce.porteous
 *
 */
public interface Instrument {

	public void process(Map<String,String> readings);

	public void drawDiagnostic(GraphicsContext gc);

	public void updated();
	
	public Node getInstrumentNode();
	
	public String[] getConfigFields();

}
