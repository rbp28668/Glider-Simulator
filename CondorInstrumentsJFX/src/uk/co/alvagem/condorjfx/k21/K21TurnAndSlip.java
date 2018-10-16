/**
 * 
 */
package uk.co.alvagem.condorjfx.k21;

import java.io.IOException;
import java.util.Map;

import javafx.scene.Group;
import javafx.scene.Node;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.paint.Color;
import javafx.scene.transform.Rotate;
import javafx.scene.transform.Scale;
import javafx.scene.transform.Translate;
import uk.co.alvagem.condorjfx.AbstractInstrument;
import uk.co.alvagem.condorjfx.Instrument;

/**
 * @author bruce.porteous
 *
 */
public class K21TurnAndSlip extends AbstractInstrument implements Instrument {

	private final static int WIDTH = 1376;
	// Centre of needle
	private final static int PCX = 26;
	private final static int PCY = 613;
	
	// Pivot point of needle on main image
	private final static int CX = 674;
	private final static int CY = 922;
	
	// location of off flag (top left)
	private final static int OFFX = 348;
	private final static int OFFY = 558;
	private Image instrument = null;
	private Image needle = null;
	private Image ball = null;
	
	private static final String MAIN_IMAGE = "resources/turn-and-slip-noball.png";
	private static final String NEEDLE_IMAGE = "resources/turn-and-slip-needle.png";
	private static final String BALL_IMAGE = "resources/turn-and-slip-ball-only.png";
	
	private double scale;	// overall scaling of images to fit. 
	
	private final Rotate needleRotate;	// of turn needle.
	private final Translate ballTranslate;
	
	private double cx; // axis of turn needle on main image.
	private double cy;
	private double pcx; // axis of turn needle on needle.
	private double pcy;
	
	private double turnrate = 0; // turn rate in rad/sec
	private double slipball = 0; // slipball deflection in rad.

	// Calibration factor to convert turn rate of radians per sec into needle angle.
	// 0.53 radians deflection corresponds to a rate 3 turn (9 degrees per sec) on the dial
	// so want to change 9 degrees per sec to 0.53
	private static final double turnScale = 0.53 / Math.toRadians(9) * Math.toDegrees(1);

	private final static Color BALL_BACK_COLOUR = Color.rgb(0x3a, 0x61, 0x8a);
	
	// background rect behind slip ball window 267,926 -> 1090, 1201
	/**
	 * 
	 */
	public K21TurnAndSlip(int width) throws IOException{
		super(width);
		
	    scale = (double)width / (double)WIDTH;
		
		instrument = createScaledImage(MAIN_IMAGE, scale);
		needle = new Image(getClass().getResourceAsStream(NEEDLE_IMAGE));
		ball = new Image(getClass().getResourceAsStream(BALL_IMAGE));
		
	    
	    cx = scale * CX;
	    cy = scale * CY;
	    pcx = scale * PCX;
	    pcy = scale * PCY;
	    
	    needleRotate = new Rotate(0,pcx,pcy);
	    // Aim to put the ball in the middle for a start.
	    ballTranslate = new Translate(  ((267+1090)/2 - ball.getWidth()/2) , (30+(926+1201)/2 - ball.getHeight()/2));
	    

	}

	/* (non-Javadoc)
	 * @see uk.co.alvagem.condorjfx.AbstractInstrument#process(java.util.Map)
	 */
	@Override
	public void process(Map<String, String> readings) {
		String val;
		val = readings.get("turnrate");
		if(val != null) {
			turnrate = Double.parseDouble(val);
		}
		val = readings.get("slipball");
		if(val != null) {
			slipball = Double.parseDouble(val);
		}
		updated();
	}

	/* (non-Javadoc)
	 * @see uk.co.alvagem.condorjfx.AbstractInstrument#updated()
	 */
	@Override
	public void updated() {
		double theta = -turnScale * turnrate; // degrees.
		if(theta < -30) {
			theta = -30;
		} else if(theta > 30) {
			theta = 30;
		}
		
		
		needleRotate.setAngle(theta);
		
		// Model slip ball as moving on a circle.  By measuring 3 points and fitting we get:
		// centre of circle (691, -142) with radius 1246)
		// Points (est centre of ball) are:
		// 944	1076
		// 394	1066
		// 676	1104
		if(slipball < -0.25){
			slipball = -0.25;
		} else if (slipball > 0.24) {
			slipball = 0.24;
		}
		
        double angle = Math.toRadians(-90) + slipball;
        double x0,y0;
        x0 = 693 + 1228 * Math.cos(angle);
        y0 = -124 - 1228* Math.sin(angle);
        x0 -= ball.getWidth()/2;
        y0 -= ball.getHeight()/2;
	    ballTranslate.setX(x0);
	    ballTranslate.setY(y0);
	}

	/* (non-Javadoc)
	 * @see uk.co.alvagem.condorjfx.AbstractInstrument#getInstrumentNode()
	 */
	@Override
	public Node getInstrumentNode() {
		ImageView instrumentView = new ImageView(instrument);
		ImageView needleView = new ImageView(needle);
		ImageView ballView = new ImageView(ball);
		
		instrumentView.setSmooth(true);
		instrumentView.setCache(true);
		
		needleView.getTransforms().addAll(
				new Translate(cx-pcx,cy-pcy),
				needleRotate,
				new Scale(scale,scale)
				);
		needleView.setSmooth(true);
		needleView.setCache(true);

		ballView.getTransforms().addAll(
				new Scale(scale,scale),
				ballTranslate
				);
		ballView.setSmooth(true);
		ballView.setCache(true);
		
		// background rect behind slip ball window 267,926 -> 1090, 1201
		final double width = 1090-267;
		final double height = 1201-926;
		Canvas c = new Canvas(width, height);
		
		GraphicsContext gc = c.getGraphicsContext2D();
		gc.setFill(BALL_BACK_COLOUR);
		gc.fillRect(0, 0, width, height);

		c.getTransforms().addAll(
				new Scale(scale,scale),
				new Translate(267,926)
				);

		Group group = new Group(c, instrumentView, needleView,ballView);

		return group;
	}
	
}
