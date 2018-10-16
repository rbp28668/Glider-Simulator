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
import javafx.scene.transform.Scale;
import javafx.scene.transform.Translate;
import uk.co.alvagem.condorjfx.AbstractInstrument;
import uk.co.alvagem.condorjfx.Instrument;

/**
 * @author bruce.porteous
 * Calibration curve to map input value to kts for the given image with kts/108 = 2pi rotation.
 * 3.25 ms -> 20kts
 * 9.45 ms -> 30kts
 * 17.05 ms -> 40kts
 * 24.70 -> 50kts
 * 31.15 -> 60kts
 * 37.0 -> 70kts
 * 42.65 -> 80kts
 * 47.75 -> 90
 * 52.85 -> 100
 * 61.9 -> 120
 * 69.8 -> 140
 * 77.4 -> 160
 */
public class K21ASIWinter extends AbstractInstrument implements Instrument {

	private static final int WIDTH = 1166;
	
	// Center of dial (pixels) as measured off the image.
	private static final int CX = 583;
	private static final int CY = 584;
	
	// Center of needle (pixels) as measured off the image.
	private static final int PCX = 48;
	private static final int PCY = 236;

	private static final String ASI_IMAGE = "resources/asi-no-needle.png";
	private static final String ASI_NEEDLE = "resources/asi-needle.png";
	
	private Image asi = null;
	private Image needle = null;
	
	//private double scale;
	
	// Center of dial (pixels) allowing for image scale.
	private double cx;
	private double cy;
	
	// Center of needle (pixels) allowing for image scale.
	private double pcx;
	private double pcy;

	private Rotate rotate;
	
	// Calibration curve of speeds vs angles for the gauge to display accurately.
	// Speeds in kts, angles in radians.
	private static double speeds[] = {
		0,
		20,
		30,
		40,
		50,
		60,
		70,
		80,
		90,
		100,
		120,
		140,
		160,
	};
	
	private static double angles[] = {
		0,
		0.367536937,
		1.068684325,
		1.928155316,
		2.793280722,
		3.522700181,
		4.184266668,
		4.823215497,
		5.399965767,
		5.976716038,
		7.000165047,
		7.893562525,
		8.753033516,
	};
	
	
	public K21ASIWinter(int width) throws IOException{
		super(width);
		this.key = "airspeed";

		double scale = (double)width / (double)WIDTH;

	    asi = createScaledImage(ASI_IMAGE, scale);
	    needle = createScaledImage(ASI_NEEDLE, scale);

		cx = scale * CX;
	    cy = scale * CY;
	    pcx = scale * PCX;
	    pcy = scale * PCY;
	    
	    rotate = new Rotate(0,pcx,pcy);
	}
	
	
	/* (non-Javadoc)
	 * @see uk.co.alvagem.condorjfx.AbstractInstrument#getInstrumentNode()
	 */
	@Override
	public Node getInstrumentNode() {
		ImageView asiView = new ImageView(asi);
		ImageView needleView = new ImageView(needle);
		
		asiView.setSmooth(true);
		asiView.setCache(true);
		
		needleView.getTransforms().addAll(
				new Translate(cx-pcx,cy-pcy),
				rotate
				);

		needleView.setSmooth(true);
		needleView.setCache(true);


		Group group = new Group(asiView, needleView);

		return group;
	}

	private double getValue() {
		double value = 0.0;
		if(this.value != null){
			value = Double.parseDouble(this.value);
			value *= 1.94384449;  // ms to kts.
		}
		return value;	
	}
	
	/* (non-Javadoc)
	 * @see uk.co.alvagem.condorjfx.AbstractInstrument#updated()
	 */
	@Override
	public void updated() {
		double kts = getValue();  
		double theta = 0;
		// 360 degrees = 108 kts approx before calibration
//		double value = kts / 108;
//		theta = value * 2 * Math.PI;

		// Calibrated:
		if(kts < 0) {
			kts = 0;
		}
		if(kts > speeds[speeds.length-1]) {
			kts = speeds[speeds.length-1];
		}
		
		// Simple linear search.
		for(int i=1; i<speeds.length; ++i){
			if(kts <= speeds[i]) {
				// kts >= speeds[i-1] & kts < speeds[i];
				
				double w = speeds[i] - speeds[i-1];
				double d0 = (kts - speeds[i-1]) / w;
				double d1 = (speeds[i] - kts) / w;
				
				theta = angles[i-1] * d1 + angles[i] * d0;
				break;
			}
		}
		
		rotate.setAngle(Math.toDegrees(theta));
	}

}
