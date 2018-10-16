/**
 * 
 */
package uk.co.alvagem.condorjfx.k21;

import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.util.Map;

import javafx.application.Application;
import javafx.stage.Stage;

import javax.imageio.ImageIO;

/**
 * Minimal JavaFX application to generate an image with the K21 compass scale.
 * Needed for RPi as it can't handle the full size of image (before scaling).
 * @author bruce.porteous
 *
 */
public class K21CompassDisplayGenerator extends Application {

	/**
	 * 
	 */
	public K21CompassDisplayGenerator() {
	}

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		launch(args);
	}

	/* (non-Javadoc)
	 * @see javafx.application.Application#start(javafx.stage.Stage)
	 */
	@Override
	public void start(Stage primaryStage) throws Exception {

		try {
			
			Parameters params = getParameters();
			Map<String,String>argProps = params.getNamed();
			String fileName = argProps.get("file");
			File tempDest = new File(fileName);
			
			K21Compass compass = new K21Compass();
			BufferedImage destImage = compass.createScaleImage();

			ImageIO.write(destImage, "PNG", tempDest);
		} catch (IOException e) {
			System.err.println("Unable to generate compass scale image");
			e.printStackTrace();
		}

		this.stop();
	}

}
