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
public class K21Altimeter extends AbstractInstrument implements Instrument {

	private static boolean isMetric = false;	// height passed as M if true, feet if false.
	
	// WIDTH of main ALTIMETER_IMAGE used to base scale on.
	private static final int WIDTH = 1238;
	
	// Center of dial (pixels) as measured off the image.
	private static final int CX = 627;
	private static final int CY = 620;
	
	// Centre of needle (pixels) as measured off the image.
	private static final int PCX_Feet = 37;
	private static final int PCY_Feet = 459;
	private static final int PCX_Thousands = 76;
	private static final int PCY_Thousands = 236;
	

	private static final String ALTIMETER_IMAGE = "resources/altimeter.png";
	private static final String ALTIMETER_NEEDLE_FEET = "resources/altimeter-needle-feet.png";
	private static final String ALTIMETER_NEEDLE_THOUSANDS = "resources/altimeter-needle-thousands.png";
	private static final String ALTIMETER_NEEDLE_TEN_THOUSANDS = "resources/altimeter-needle-ten-thousands.png";
	
	private Image altimeter = null;
	private Image needleFeet = null;
	private Image needleThousands = null;
	private Image needleTenThousands = null;
	
	// Scaling factor for all the images.
	//double scale = 1.0;
	
	// Centre of dial (pixels) allowing for image scale.
	private double cx;
	private double cy;
	
	// Centre of needle (pixels) allowing for image scale.
	private double pcxFeet;
	private double pcyFeet;
	private double pcxThousands;
	private double pcyThousands;

	private Rotate feetRotate;
	private Rotate thousandsRotate;
	
	
	public K21Altimeter(int width) throws IOException{
		super(width);
		this.key = "altitude"; // in m
		
		double scale = (double)width / (double)WIDTH;
		
	    altimeter = createScaledImage(ALTIMETER_IMAGE, scale);
	    needleFeet = createScaledImage(ALTIMETER_NEEDLE_FEET, scale);
	    needleThousands = createScaledImage(ALTIMETER_NEEDLE_THOUSANDS, scale);
		
	    cx = scale * CX;
	    cy = scale * CY;
	    pcxFeet = scale * PCX_Feet;
	    pcyFeet = scale * PCY_Feet;
	    pcxThousands = scale * PCX_Thousands;
	    pcyThousands = scale * PCY_Thousands;

	    feetRotate = new Rotate(0,pcxFeet,pcyFeet);
	    thousandsRotate = new Rotate(0,pcxThousands,pcyThousands);
	}
	
	
	/* (non-Javadoc)
	 * @see uk.co.alvagem.condorjfx.AbstractInstrument#getInstrumentNode()
	 */
	@Override
	public Node getInstrumentNode() {
		
		ImageView altimeterView = new ImageView(altimeter);
		ImageView needleFeetView = new ImageView(needleFeet);
		ImageView needleThousandsView = new ImageView(needleThousands);
		
//		altimeterView.getTransforms().addAll(
//				new Scale(scale,scale)
//				);
		
		altimeterView.setSmooth(true);
		altimeterView.setCache(true);
		
		needleFeetView.getTransforms().addAll(
				new Translate(cx-pcxFeet,cy-pcyFeet),
				feetRotate
//				new Scale(scale,scale)
				);

		needleFeetView.setSmooth(true);
		needleFeetView.setCache(true);

		needleThousandsView.getTransforms().addAll(
				new Translate(cx-pcxThousands,cy-pcyThousands),
				thousandsRotate
//				new Scale(scale,scale)
				);

		needleThousandsView.setSmooth(true);
		needleThousandsView.setCache(true);

		Group group = new Group(altimeterView, needleThousandsView, needleFeetView);

		return group;
	}
	
	/* (non-Javadoc)
	 * @see uk.co.alvagem.condorjfx.AbstractInstrument#updated()
	 */
	@Override
	public void updated() {
		double value = getValue();

		double angleFeet = value / 1000.0 * 2.0 * Math.PI;
		double angleThousands = value / 10000.0 * 2.0 * Math.PI;
		//double angleTenThousands = value / 100000.0 * 2.0 * Math.PI;
		
		angleFeet = Math.toDegrees(angleFeet);
		angleThousands = Math.toDegrees(angleThousands);
		
		feetRotate.setAngle(angleFeet); 
		thousandsRotate.setAngle(angleThousands);
	}

	private double getValue() {
		double value = 0.0;
		
		if(this.value != null){
			value = Double.parseDouble(this.value);
			if(isMetric){
				value *= 3.2808399;  // meters to feet
			}
		}
		return value;	
	}





}
