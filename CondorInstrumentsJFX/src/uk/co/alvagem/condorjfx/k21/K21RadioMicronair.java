/**
 * 
 */
package uk.co.alvagem.condorjfx.k21;

import java.io.IOException;

import javafx.scene.Group;
import javafx.scene.Node;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.transform.Scale;
import javafx.scene.transform.Translate;
import uk.co.alvagem.condorjfx.AbstractInstrument;
import uk.co.alvagem.condorjfx.Instrument;

/**
 * @author bruce.porteous
 *
 */
public class K21RadioMicronair extends AbstractInstrument implements Instrument {

	private final static int WIDTH = 1188;
	
	private Image instrument = null;
	private MicronairDisplay display = new MicronairDisplay();
	
	private static final String MAIN_IMAGE = "resources/radio-nodisplay.png";
	private double scale;
	
	private String standby = "121.500";
	private String active = "131.275";

	/**
	 * 
	 */
	public K21RadioMicronair(int width) throws IOException {
		super(width);
		this.key = "radiofrequency";
		scale = (double)width / (double)WIDTH;
		instrument = createScaledImage(MAIN_IMAGE, scale);
	}

	/* (non-Javadoc)
	 * @see uk.co.alvagem.condorjfx.AbstractInstrument#updated()
	 */
	@Override
	public void updated() {
		if(this.value != null) {
			if(!this.value.equals(active)) {
				standby = active;
				active = this.value;
				display.update(active, standby);
			}
		}
		
	}

	/* (non-Javadoc)
	 * @see uk.co.alvagem.condorjfx.AbstractInstrument#getInstrumentNode()
	 */
	@Override
	public Node getInstrumentNode() {
		ImageView instrumentView = new ImageView(instrument);
		instrumentView.setSmooth(true);
		instrumentView.setCache(true);
		
		Node n = display.createNode(active, standby);
		n.getTransforms().addAll(
				new Scale(scale,scale),
				new Translate(244,447)
				);
		
		Group group = new Group(n,instrumentView);

		return group;
	}

}
