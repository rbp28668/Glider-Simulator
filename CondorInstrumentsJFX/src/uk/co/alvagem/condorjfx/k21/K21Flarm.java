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
import uk.co.alvagem.condorjfx.AbstractInstrument;
import uk.co.alvagem.condorjfx.Instrument;

/**
 * @author bruce.porteous
 *
 */
public class K21Flarm extends AbstractInstrument implements Instrument {

	private final static int WIDTH = 1700;
	private Image instrument = null;
	private static final String MAIN_IMAGE = "resources/flarm-complete.png";
	
	/**
	 * 
	 */
	public K21Flarm(int width) throws IOException {
		super(width);
		double scale = (double)width / (double)WIDTH;
		instrument = createScaledImage(MAIN_IMAGE,scale);
	    
	}

	/* (non-Javadoc)
	 * @see uk.co.alvagem.condorjfx.AbstractInstrument#updated()
	 */
	@Override
	public void updated() {
		
	}

	/* (non-Javadoc)
	 * @see uk.co.alvagem.condorjfx.AbstractInstrument#getInstrumentNode()
	 */
	@Override
	public Node getInstrumentNode() {
		ImageView instrumentView = new ImageView(instrument);
		
		instrumentView.setSmooth(true);
		instrumentView.setCache(true);
		
		Group group = new Group(instrumentView);

		return group;
	}

}
