/**
 * 
 */
package uk.co.alvagem.condorjfx;

import java.awt.Graphics2D;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Map;

import javax.imageio.ImageIO;

import javafx.embed.swing.SwingFXUtils;
import javafx.scene.Node;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.paint.Color;

/**
 * @author bruce.porteous
 *
 */
public class AbstractInstrument implements Instrument {

	protected String key = "unknown";
	protected String value = null;
	protected int width = 0;
	
	
	public AbstractInstrument(int width) {
		this.width = width;
	}
	
	/* (non-Javadoc)
	 * @see uk.co.alvagem.condorjfx.Instrument#process(java.util.Map)
	 */
	@Override
	public void process(Map<String, String> readings) {
		value = readings.get(key);
		updated();
	}


	/* (non-Javadoc)
	 * @see uk.co.alvagem.condorjfx.Instrument#drawDiagnostic(javafx.scene.canvas.GraphicsContext)
	 */
	@Override
	public void drawDiagnostic(GraphicsContext gc) {
		gc.setStroke(Color.RED);
		gc.strokeLine(-20, 0, 20, 0);
		gc.strokeLine(0,-20,0,20);
	}


	/* (non-Javadoc)
	 * @see uk.co.alvagem.condorjfx.Instrument#updated()
	 */
	@Override
	public void updated() {
	}


	/* (non-Javadoc)
	 * @see uk.co.alvagem.condorjfx.Instrument#getInstrumentNode()
	 */
	@Override
	public Node getInstrumentNode() {
		Canvas c = new Canvas();
		return c;
	}


	/* (non-Javadoc)
	 * @see uk.co.alvagem.condorjfx.Instrument#getConfigFields()
	 */
	@Override
	public String[] getConfigFields() {
		String[] fields =  { "width" };
		return fields;
	}
	
	public int getWidth() {
		return width;
	}

	/**
	 * Creates a scaled version of an image based on an initial resource and a scale factor.
	 * The image is returned as an InputStream from which the image can be read.
	 * @param resource is the classpath relative resource that contains the source image.
	 * @param scale is the desired scale factor.
	 * @return The scaled image.
	 * @throws IOException
	 */
	protected javafx.scene.image.Image createScaledImage(String resource, double scale) throws IOException {
		File dest = getCachedPath(resource);
		
		if(!dest.exists()) {
			BufferedImage sourceImage = null;
			try (InputStream is = getClass().getResourceAsStream(resource)) {
				sourceImage = ImageIO.read(is);
			}
			
			java.awt.Image scaledImage = scaleImage(sourceImage, scale);
			BufferedImage destImage = asBufferedImage(scaledImage);
			ImageIO.write(destImage, "PNG", dest);
		}
		
		System.out.println("Using " + dest.getAbsolutePath());
		assert(dest.exists());
		
		try(FileInputStream fis = new FileInputStream(dest)){
			return new javafx.scene.image.Image(fis);
		}
	}

	/**
	 * @param scaledImage
	 * @return
	 */
	protected BufferedImage asBufferedImage(java.awt.Image scaledImage) {
		BufferedImage destImage = new BufferedImage(scaledImage.getWidth(null), scaledImage.getHeight(null), BufferedImage.TYPE_INT_ARGB);
		Graphics2D g2d = destImage.createGraphics();
		g2d.drawImage(scaledImage, 0, 0, null);
		return destImage;
	}
	

	/**
	 * Based on a resource and the desired width creates a path to a cached scaled
	 * copy of the image.  Sub-directories are created as necessary to ensure that
	 * similarly named resources in different folders remain unique.
	 * The paths are rooted in the users temp directory.
	 * @param resource is the resource to create a path for.
	 * @return a File containing the path to hold a scaled copy of resource.
	 */
	protected File getCachedPath(String resource) {
		int idx = resource.lastIndexOf("/");
		String name = resource;
		if(idx != -1) {
			name = resource.substring(idx + 1);
		}
		name = Integer.toString(width) + name;
		
		File tmpDir = new File(System.getProperty("java.io.tmpdir"));
		File subDir = new File(tmpDir,getClass().getPackage().getName());
		subDir.mkdirs();
		
		File path = new File(subDir, name);
		
		return path;
	}
	

	/**
	 * Gets a scaled copy of an image.
	 * @param source is the source image.
	 * @param scale is the scale factor.
	 */
	protected java.awt.Image scaleImage(java.awt.Image source, double scale) {
		java.awt.Image scaled = source.getScaledInstance((int)(Math.round(scale * source.getWidth(null))), 
	    		(int)(Math.round(scale * source.getHeight(null))), java.awt.Image.SCALE_SMOOTH);
		return scaled;
	}

}
