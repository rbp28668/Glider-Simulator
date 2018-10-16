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
 *
 */
public class K21GMeter extends AbstractInstrument implements Instrument {

	private static final int WIDTH = 1582;
	
	private Image instrument = null;
	private Image mainNeedle = null;
	private Image auxNeedle = null;
	
	private static final String MAIN_IMAGE = "resources/g-meter.png";
	private static final String NEEDLE_IMAGE = "resources/g-meter-main-needle.png";
	private static final String AUX_IMAGE = "resources/g-meter-aux-needle.png";
	
	private static final int CX = 820;
	private static final int CY = 786;

	private static final int PCX_NEEDLE = 597;
	private static final int PCY_NEEDLE = 106;

	private static final int PCX_AUX = 562;
	private static final int PCY_AUX = 103;
	
	private double cx;
	private double cy;
	private double pcxNeedle;
	private double pcyNeedle;
	private double pcxAux;
	private double pcyAux;
	
	private Rotate rotateMain;
	private Rotate rotateMin;
	private Rotate rotateMax;
	
	private double minValue = 1.0;
	private double maxValue = 1.0;
	private long lastUpdateTime = 0;
	
	private final static double offset = 1.0; // 1 g is horizontal
	private final static double cal = 45.0;  // 3 g is 90 degrees (2 g increment)
	/**
	 * 
	 */
	public K21GMeter(int width) throws IOException {
		super(width);
		this.key = "g";
		
		double scale = (double)width / (double)WIDTH;

		instrument = createScaledImage(MAIN_IMAGE, scale);
		mainNeedle = createScaledImage(NEEDLE_IMAGE, scale);
		auxNeedle = createScaledImage(AUX_IMAGE, scale);
	    
	    cx = scale * CX;
	    cy = scale * CY;
	    pcxNeedle = scale * PCX_NEEDLE;
	    pcyNeedle = scale * PCY_NEEDLE;
	    pcxAux = scale * PCX_AUX;
	    pcyAux = scale * PCY_AUX;

	    
	    rotateMain = new Rotate(0,pcxNeedle,pcyNeedle);
	    rotateMin = new Rotate(0,pcxAux,pcyAux);
	    rotateMax = new Rotate(0,pcxAux,pcyAux);

	}

	/* (non-Javadoc)
	 * @see uk.co.alvagem.condorjfx.AbstractInstrument#updated()
	 */
	@Override
	public void updated() {
		long now = System.currentTimeMillis();
		if(now - lastUpdateTime > 5000) {
			minValue = maxValue = 1.0;
		}
		lastUpdateTime = now;
		
		double value = getValue();
		
		minValue = Math.min(value, minValue);
		maxValue = Math.max(value, maxValue);
		
		rotateMain.setAngle((value-offset) * cal);
		rotateMin.setAngle((minValue-offset) * cal);
		rotateMax.setAngle((maxValue-offset) * cal);
	
	}
	
	private double getValue() {
		double value = 1.0;
		if(this.value != null){
			value = Double.parseDouble(this.value);
		}
		return value;	
	}


	/* (non-Javadoc)
	 * @see uk.co.alvagem.condorjfx.AbstractInstrument#getInstrumentNode()
	 */
	@Override
	public Node getInstrumentNode() {

		ImageView instrumentView = new ImageView(instrument);
		instrumentView.setSmooth(true);
		instrumentView.setCache(true);

		ImageView needleView = new ImageView(mainNeedle);
		needleView.getTransforms().addAll(
				new Translate(cx-pcxNeedle,cy-pcyNeedle),
				rotateMain
				);
		needleView.setSmooth(true);
		needleView.setCache(true);

		ImageView minView = new ImageView(auxNeedle);
		minView.getTransforms().addAll(
				new Translate(cx-pcxAux,cy-pcyAux),
				rotateMin
				);
		minView.setSmooth(true);
		minView.setCache(true);

		ImageView maxView = new ImageView(auxNeedle);
		maxView.getTransforms().addAll(
				new Translate(cx-pcxAux,cy-pcyAux),
				rotateMax
				);
		maxView.setSmooth(true);
		maxView.setCache(true);

		Group group = new Group(instrumentView,minView,maxView, needleView);

		return group;
	}

}
