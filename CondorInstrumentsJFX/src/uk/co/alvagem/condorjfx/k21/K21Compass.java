/**
 * 
 */
package uk.co.alvagem.condorjfx.k21;

import java.awt.image.BufferedImage;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

import javafx.embed.swing.SwingFXUtils;
import javafx.scene.Group;
import javafx.scene.Node;
import javafx.scene.Scene;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.image.WritableImage;
import javafx.scene.paint.Color;
import javafx.scene.shape.Rectangle;
import javafx.scene.shape.StrokeLineCap;
import javafx.scene.text.Font;
import javafx.scene.transform.Translate;

import javax.imageio.ImageIO;

import uk.co.alvagem.condorjfx.AbstractInstrument;
import uk.co.alvagem.condorjfx.Instrument;

import com.sun.javafx.tk.FontMetrics;
import com.sun.javafx.tk.Toolkit;

/**
 * @author bruce.porteous
 *
 */
public class K21Compass extends AbstractInstrument implements Instrument {

	private final static int WIDTH = 1260; // raw width of compass.png
	
	private Image instrument = null; // overall instrument image
	private Image scaleImage = null; // artificial image of scale
	
	private static final String MAIN_IMAGE = "resources/compass.png";
	private static final String SCALE_IMAGE = "resources/compass-scale.png";
	
	private double scale;
	
	private final static Color BACKGROUND_COLOUR = Color.rgb(0x39, 0x4a,0x59);
	private final static Color DIGIT_COLOUR = Color.rgb(0xa6,0x8f, 0x63);
	
	private final Translate scaleTranslate;
	private final Rectangle clipRect;

	private final static double degreesToPixels = 250.0 / 30.0;  // measured 30 degrees as 250 pixels off compass.png

	K21Compass() throws IOException {
		super(0);
		scaleTranslate = new Translate(0,0);
		clipRect = new Rectangle(0,0, 0,0);
	}
	
	/**
	 * Creates a compass of a given width.
	 * @param width is the width in pixels for the final compass. All the images
	 * and the compass scale are scaled by a corresponding amount.
	 */
	public K21Compass(int width) throws IOException {
		super(width);
		this.key = "compass";
		scale = (double)width / (double)WIDTH;

		instrument = createScaledImage(MAIN_IMAGE, scale);
		scaleImage = createScaledImage(SCALE_IMAGE,scale); // was getScaleImage(scale);
		
		scaleTranslate = new Translate(222 * scale, 234 * scale);
		clipRect = new Rectangle(0,0, (1043-222) * scale, (900-234) * scale);

	}

	/* (non-Javadoc)
	 * @see uk.co.alvagem.condorjfx.AbstractInstrument#updated()
	 */
	@Override
	public void updated() {
		
		double offset = 222; // LHS of clip window
		
		// zero degrees is at (360 + 180) * degreesToPixels. To read zero
		// this must be in the centre (at x=629 pixels).  As heading increases
		// we need to shift the scale image right.
		double x = ((360+180) - getValue()) * degreesToPixels - 629 ; 
		scaleTranslate.setX(-x * scale);
		clipRect.setTranslateX((x+offset) * scale);
	}
	
	private double getValue() {
		double value = 0;
		if(this.value != null) {
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

		ImageView scaleView = new ImageView(scaleImage);
		scaleView.setSmooth(true);
		scaleView.setCache(true);
		scaleView.getTransforms().add(scaleTranslate);
		scaleView.setClip(clipRect);
		
//		Canvas c = drawScale();
//		
//		c.getTransforms().addAll(
//				new Scale(scale,scale),
//				scaleTranslate
//				);
//		c.setClip(clipRect);		
		
	
		Group group = new Group(scaleView,instrumentView);

		return group;
	}



	// Major degree ticks, 30 degrees, 250 pixels, 17pixels wide, 64 high.
	// 10 degree ticks 12 pixels wide, 64 high
	// 5 degree ticks 12 pixels wide, 31 pixels high
	// digits 98 pixels high 63 pixels wide.
	// Letters NESW for cardinal points.  numeric heading on other major ticks.
	// 9 pixel line right down the middle.  x=629 pixels. (image down the middle).
	
	/**
	 * Draw the scale of the compass.
	 * Note this draws the scale at 1:1 scale with the original unscaled image.
	 * @return a Canvas with the compass scale drawn on it.
	 */
	private Canvas drawScale() {


		// background rect behind  window 222,234 -> 1043,900
		// width though we want to allow for 720 degrees of compass card to ensure we
		// don't go off the end.
		final double width = 720 * degreesToPixels;
		final double height = 900-234;
		Canvas c = new Canvas(width, height);
		
		GraphicsContext gc = c.getGraphicsContext2D();
		gc.setFill(BACKGROUND_COLOUR);
		gc.fillRect(0, 0, width, height);
		

		gc.setFill(DIGIT_COLOUR);	// for text
		gc.setStroke(DIGIT_COLOUR); // and lines
		gc.setLineCap(StrokeLineCap.ROUND);
		
		double baseline = 380; //335;
		
		// Thin horizontal line on baseline.  measured at c. 5 pixels high.
		gc.setLineWidth(5.0);
		gc.strokeLine(0, baseline, width, baseline);
		

		Font f = new Font("Arial Rounded MT Bold",128);
		gc.setFont(f);
		Toolkit tk = Toolkit.getToolkit();
		FontMetrics fm = tk.getFontLoader().getFontMetrics(f);

		
		final double y = baseline - 20; //
		for(int id = -180; id <= (360 + 180); id += 5) {  // marker every 5 degrees;
			
			double x = (id + 180) * degreesToPixels;
			int degrees = (id + 360) % 360;
			
			if((degrees % 30) == 0) {
				gc.setLineWidth(17);
				gc.strokeLine(x, y, x, y - 64);
				
				String label;
				int hdg = 360-degrees; // compass cards read backwards
				if((degrees % 90) == 0){
					int idx = (hdg / 90) % 4;
					char[] ch = new char[1];
					ch[0] = "NESW".charAt(idx);
					label = new String(ch);
				} else {
					label = Integer.toString(hdg/10);
				}
				float sw = fm.computeStringWidth(label);

				gc.fillText(label, x-sw/2, y-100);
				
			} else if((degrees % 10) == 0) {
				gc.setLineWidth(12);
				gc.strokeLine(x, y, x, y - 64);
			} else {
				gc.setLineWidth(12);
				gc.strokeLine(x, y, x, y - 31);
			}
		}
		
		return c;
	}
	
	/**
	 * Generates an image of the compass scale in the standard image cache.
	 * @param scale is the scale factor to use to scale down the size that
	 * would match the original images to the size we need to match the desired
	 * width.
	 * @return a correctly scaled image.
	 * @throws IOException
	 */
	private Image getScaleImage(double scale) throws IOException {

		File destFile = getCachedPath("compass-scale.png");
		if(!destFile.exists()){
			writeCanvasImage(destFile, scale);
		}
		System.out.println("Using " + destFile.getAbsolutePath());
		try (FileInputStream fis = new FileInputStream(destFile)) {
			return new Image(fis);
		}
	}
	
	/**
	 * Generates, and writes out an image of the compass scale scaled to match
	 * the scale of the source image. 
	 * @param destFile
	 * @param scale
	 * @return
	 * @throws IOException
	 */
	private File writeCanvasImage(File destFile, double scale) throws IOException{
		
		Group g = new Group();
		Canvas c = drawScale();
		g.getChildren().add(c);
		
		Scene scene = new Scene(g);
		
		WritableImage img = scene.snapshot(null);
		BufferedImage destImage = asBufferedImage(scaleImage(SwingFXUtils.fromFXImage(img, null), scale));
		ImageIO.write(destImage, "PNG", destFile);

		return destFile;                                                                                                                                                                   
	}
	
	/**
	 * Helper method to allow creation of a full size compass scale that can be treated
	 * as any other image.
	 * @return
	 */
	BufferedImage createScaleImage() {
		Group g = new Group();
		Canvas c = drawScale();
		g.getChildren().add(c);
		
		Scene scene = new Scene(g);
		
		WritableImage img = scene.snapshot(null);
		BufferedImage destImage = asBufferedImage(SwingFXUtils.fromFXImage(img, null));
		return destImage;

	}

}
