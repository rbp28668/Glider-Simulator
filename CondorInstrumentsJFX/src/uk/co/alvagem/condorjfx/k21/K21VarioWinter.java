/**
 * 
 */
package uk.co.alvagem.condorjfx.k21;


import java.io.IOException;

import javafx.scene.Group;
import javafx.scene.Node;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.transform.Rotate;
import javafx.scene.transform.Translate;
import uk.co.alvagem.condorjfx.AbstractInstrument;
import uk.co.alvagem.condorjfx.Instrument;

/**
 * @author bruce.porteous
 */
public class K21VarioWinter extends AbstractInstrument implements Instrument {

	private static final int WIDTH = 1294;
	
	private static final double MAX_ENDSTOP = 10;
	private static final double MIN_ENDSTOP = -10;

	// Center of dial (pixels) as measured off the image.
	private static final int CX = 641;
	private static final int CY = 649;
	
	// Center of needle (pixels) as measured off the image.
	private static final int PCX = 478;
	private static final int PCY = 13;

	private static final String VARIO_IMAGE = "resources/vario-winter.png";
	private static final String VARIO_NEEDLE = "resources/vario-winter-needle.png";
	
	private Image vario = null;
	private Image needle = null;
	
	// Center of dial (pixels) allowing for image scale.
	private double cx;
	private double cy;
	
	// Center of needle (pixels) allowing for image scale.
	private double pcx;
	private double pcy;

	private Rotate rotate;
	
	public K21VarioWinter(int width) throws IOException{
		super(width);
		this.key = "vario";
		
		double scale = (double)width / (double)WIDTH;
	    vario = createScaledImage(VARIO_IMAGE, scale);
	    needle = createScaledImage(VARIO_NEEDLE, scale);
	    
	    cx = scale * CX;
	    cy = scale * CY;
	    pcx = scale * PCX;
	    pcy = scale * PCY;
	    
	    rotate = new Rotate(0,pcx,pcy);
	}
	/* (non-Javadoc)
	 * @see uk.co.alvagem.condorjfx.AbstractInstrument#updated()
	 */
	@Override
	public void updated() {
		rotate.setAngle(getAngle());
	}

	/* (non-Javadoc)
	 * @see uk.co.alvagem.condorjfx.AbstractInstrument#getInstrumentNode()
	 */
	@Override
	public Node getInstrumentNode() {
		ImageView varioView = new ImageView(vario);
		ImageView needleView = new ImageView(needle);
		
		varioView.setSmooth(true);
		varioView.setCache(true);
		
		needleView.getTransforms().addAll(
				new Translate(cx-pcx,cy-pcy),
				rotate
				);

		needleView.setSmooth(true);
		needleView.setCache(true);


		Group group = new Group(varioView, needleView);

		return group;
	}

	private double getValue() {
		double value = 0.0;
		if(this.value != null){
			try {
				value = Double.parseDouble(this.value);
				value *= 1.94384449;  // ms to kts.
				if(value > MAX_ENDSTOP) {
					value = MAX_ENDSTOP;
				} else if (value < MIN_ENDSTOP){
					value = MIN_ENDSTOP;
				}
			} catch (NumberFormatException e) {
				System.out.println("Invalid  value for Winger Vario: " + e.getMessage());
			}
		}
		return value;	
	}
	
	private double getAngle() {
		double kts = getValue();  
		// + 10 kts is angle of 135 degrees cw (within 0.2 degree measured)
		// - 10 kts is angle of 135 degrees cc
		// Slight rotation of dial so 1.2 degree offset.
		double theta = (1.2 + kts * 13.5) * Math.PI / 180; // in radians.
		return Math.toDegrees(theta);
	}

}
