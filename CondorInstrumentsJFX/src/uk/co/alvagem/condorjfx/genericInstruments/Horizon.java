/**
 * 
 */
package uk.co.alvagem.condorjfx.genericInstruments;

import java.util.Map;

import javafx.scene.Group;
import javafx.scene.Node;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.paint.Color;
import javafx.scene.shape.ArcType;
import javafx.scene.shape.Rectangle;
import javafx.scene.shape.StrokeLineCap;
import javafx.scene.transform.Rotate;
import javafx.scene.transform.Translate;
import uk.co.alvagem.condorjfx.AbstractInstrument;
import uk.co.alvagem.condorjfx.Instrument;

/**
 * Draws an artificial horizon based on a Dynon D10 image 
 * Dynon D10 image 800 x 664 pixels overall with 94 to 702 width screen, 95 to 514 height.  
 * "Magic numbers" used for dimensions in the code are measured of an image, as are colours.
 * Note P3D parameters:
 * PLANE PITCH DEGREES	Pitch angle, although the name mentions degrees the units used are radians	Radians	Y	-
 * PLANE BANK DEGREES	Bank angle, although the name mentions degrees the units used are radians	Radians	Y	-
 * ATTITUDE INDICATOR PITCH DEGREES	AI pitch indication	Radians	N	-
 * ATTITUDE INDICATOR BANK DEGREES	AI bank indication	Radians	N	-
 * @author bruce.porteous
 *
 */
public class Horizon extends AbstractInstrument implements Instrument {

	private final static Color BACKGROUND_COLOUR = Color.rgb(43,43,45);
	private final static Color SKY_COLOUR = Color.rgb(36,64,187);
	private final static Color GROUND_COLOUR = Color.rgb(103,104,46);
	private final static Color TEXT_COLOUR = Color.rgb(254,254,254);
	
	
    private final float scale; // scale measured pixels to width given.
	private final float degreesToPixels;
    private double pitch;
	private double roll;
	
	private Translate horizonTranslate = new Translate(0,0);
	private Rotate horizonRotate = new Rotate(0);
	
	/**
	 * @param width
	 */
	public Horizon(int width) {
		super(width);

		scale = (float)width / 800;
		degreesToPixels = (366-296) * scale * 0.1f;
		horizonTranslate.setY(-90 * degreesToPixels); // initialise to 0 pitch (halfway down the scale)
	}

	/* (non-Javadoc)
	 * @see uk.co.alvagem.condorjfx.AbstractInstrument#process(java.util.Map)
	 */
	@Override
	public void process(Map<String, String> readings) {
		String val = readings.get("pitch");
		if(val != null) {
			pitch = Double.parseDouble(val);
		}
		val = readings.get("bank");
		if(val != null) {
			roll = Double.parseDouble(val);
		}
		updated();
	}

	/* (non-Javadoc)
	 * @see uk.co.alvagem.condorjfx.AbstractInstrument#updated()
	 */
	@Override
	public void updated() {

		double yOffset = (Math.toDegrees(-pitch) - 90) * degreesToPixels;
		horizonTranslate.setY(yOffset);
		horizonRotate.setAngle(Math.toDegrees(roll));
	}

	/* (non-Javadoc)
	 * @see uk.co.alvagem.condorjfx.AbstractInstrument#getInstrumentNode()
	 */
	@Override
	public Node getInstrumentNode() {
		int width = getWidth();
		int height = (width * 664)/800;
		
		horizonRotate.setPivotX(width/2);
		horizonRotate.setPivotY(height/2);
		Canvas fg = drawFixedForeground(width,height);
		Canvas r = drawRollIndicator(width,height);
		r.getTransforms().add(horizonRotate);
		
		Canvas h = drawHorizon(width,height);
		h.getTransforms().add(horizonRotate);
		h.getTransforms().add(horizonTranslate);
		Group hg = new Group(h);  // put main horizon in group so we can set constant clip rect irrespective of underlying transforms.
    	Rectangle clipRect = new Rectangle(94*scale,95*scale, (702-94) * scale, (514-95) * scale);
		hg.setClip(clipRect);

		Canvas bg = drawBackground(width,height);
		Group group = new Group(bg, hg, r, fg);

		return group;
	}

	/**
	 * Draws the bottom layer of the stack.
	 * @param width
	 * @param height
	 * @return
	 */
	private Canvas drawBackground(int width, int height) {
		Canvas c = new Canvas(width, height);
		
		GraphicsContext gc = c.getGraphicsContext2D();
		gc.setFill(BACKGROUND_COLOUR);
		gc.fillRect(0, 0, width, height);
		return c;
	}
	
	/**
	 * Draws everything that is locked to the horizon and that hence has
	 * rotation for roll and translation for pitch.  Tries to draw a long
	 * strip so that we get +/- 90 degrees for pich so must be clipped
	 * by its parent.
	 * @param width
	 * @param height
	 * @return
	 */
	private Canvas drawHorizon(int width,int height) {
		int len = (int)(90 * degreesToPixels);
		Canvas c = new Canvas(width, height+2*len);
		
		int centre = height/2 + len;
		
		GraphicsContext gc = c.getGraphicsContext2D();
		gc.setFill(SKY_COLOUR);
		gc.fillRect(0, 0, width, centre);
		gc.setFill(GROUND_COLOUR);
		gc.fillRect(0, centre, width, centre);
		
		setNotationColours(gc);
		
		// Thin horizontal line separating sky and land
		gc.setLineWidth(1.0);
		gc.strokeLine(0, centre, width, centre);

		// size of image features measured from dynon image
		final int smallPitchLine = Math.round((444-372) * scale); 
		final int largePitchLine = Math.round((466-324) * scale);
		final int pitchTicks = Math.round((358-341) * scale);
		
		int mid = width/2;
		for(int degrees = 10; degrees <=90; degrees += 10) {
			int dy = Math.round((degrees - 5) * degreesToPixels);
			int l = mid-smallPitchLine/2;
			int r = mid+smallPitchLine/2;
			gc.strokeLine(l,  centre+dy, r, centre+dy);
			gc.strokeLine(l,  centre-dy, r, centre-dy);

			dy = Math.round((degrees) * degreesToPixels);
			l = mid-largePitchLine/2;
			r = mid+largePitchLine/2;
			gc.strokeLine(l,  centre+dy, r, centre+dy);
			gc.strokeLine(l,  centre-dy, r, centre-dy);
			
			gc.strokeLine(l, centre+dy, l, centre+dy-pitchTicks);
			gc.strokeLine(r, centre+dy, r, centre+dy-pitchTicks);
			gc.strokeLine(l, centre-dy, l, centre-dy+pitchTicks);
			gc.strokeLine(r, centre-dy, r, centre-dy+pitchTicks);
		}
		
		
		return c;

	}

	/**
	 * Layer on top of the main horizon - this shows roll irrespective of pitch
	 * Create a 60 degree arc with angle of bank indicators aligned with the horizon (i.e. level)
	 * Markers at +/10, 20,30, 45 and 60 degrees.  30 and 60 long, others about half.
	 * @param width
	 * @param height
	 * @return
	 */
	private Canvas drawRollIndicator(int width, int height) {
		Canvas c = new Canvas(width, height);
		
		final double cy = height/2;
		final double cx = width/2;
		
		GraphicsContext gc = c.getGraphicsContext2D();
		setNotationColours(gc);
		
		drawRollPointer(cx, cy, gc);
				
		return c;

	}


	/**
	 * Draws foreground symbology that's fixed in place
	 * @param width
	 * @param height
	 * @return
	 */
	private Canvas drawFixedForeground(int width,int height) {
		Canvas c = new Canvas(width, height);
		
		GraphicsContext gc = c.getGraphicsContext2D();
		setNotationColours(gc);

		final double cx = width/2;
		final double cy = height/2;
		
		
		drawRollScale(cx, cy, gc);
		drawAircraftSymbol(cx, cy, gc);
		
		return c;

	}

	/**
	 * @param gc
	 */
	private void setNotationColours(GraphicsContext gc) {
		gc.setStroke(TEXT_COLOUR); // and lines
		gc.setFill(TEXT_COLOUR);
		gc.setLineCap(StrokeLineCap.ROUND);
	}


	/**
	 * @param cx is the X coordinate of the centre of the display
	 * @param cy is the Y coordinate of the centre of the display
	 * @param gc is the graphics context to draw on.
	 */
	private void drawAircraftSymbol(double cx, double cy, GraphicsContext gc) {
		// Aeroplane symbol
		final double size = 32*scale;
		final double s2 = size/2;
		
		gc.setLineWidth(4*scale);
		gc.strokeOval(cx-s2, cy-s2, size,size);
		
		final double len = 41*scale;
		gc.setLineWidth(6*scale);
		gc.strokeLine(cx - s2 - len, cy, cx - s2, cy);
		gc.strokeLine(cx + s2, cy, cx + s2 + len, cy);
		
		final double fin = 22*scale;
		gc.setLineWidth(4*scale);
		gc.strokeLine(cx, cy-s2, cx, cy-s2-fin);
	}

	/**
	 * This draws the semi-circular roll scale
	 * @param cx is the centre X coordinate that forms the centre of the arc.
	 * @param cy is the corresponding Y coordinate.
	 * @param gc is the graphics context to draw into.
	 */
	private  void drawRollScale(double cx, double cy, GraphicsContext gc) {

		gc.setStroke(TEXT_COLOUR);
		final float rollRadius = (339-219) * scale;
		final float rollTicks = 16*scale;
		
		gc.strokeArc(cx-rollRadius, cy-rollRadius, 2*rollRadius, 2*rollRadius, 30, 120, ArcType.OPEN);
		
		final int angles[] = {-90, -60,-45,-30,-20,-10, 0, 10, 20, 30, 45, 60, 90}; // degrees either side of centre
		final int lengths[]= { 1,   1,  0,  1,  0,  0,  1, 0,  0,  1,  0,  1,  1}; // long or short ticks.
		
		for(int i=0; i<angles.length; ++i) {
			double len = rollTicks * (lengths[i] +1)/2;
			
			double cos = Math.cos(Math.toRadians(90 + angles[i]));
			double sin = Math.sin(Math.toRadians(90 + angles[i]));
			double x0 = cx + rollRadius*cos;
			double y0 = cy - rollRadius*sin;
			double x1 = cx + (rollRadius + len)*cos;
			double y1 = cy - (rollRadius + len)*sin;
			gc.strokeLine(x0, y0, x1, y1);
		}
	}
	
	/**
	 * Draws the triangular roll pointer.
	 * @param cx is the centre X coordinate of the instrument around which symbols rotate
	 * @param cy is the centre Y coordinate of the instrument
	 * @param gc is the graphics context.
	 */
	private  void drawRollPointer(double cx, double cy, GraphicsContext gc) {
		// Triangle roll pointer
		final double rollRadius = (339-219) * scale;
		final double rollTicks = 16*scale;

		double[] xPoints = new double[3];
		double[] yPoints = new double[3];
		
		double x = cx;
		double y = cy - rollRadius - rollTicks;
		
		xPoints[0] = x;
		yPoints[0] = y;
		xPoints[1] = x - rollTicks/2;
		yPoints[1] = y - rollTicks;
		xPoints[2] = x + rollTicks/2;
		yPoints[2] = y - rollTicks;
		
		gc.fillPolygon(xPoints,yPoints,3);
	}

}
