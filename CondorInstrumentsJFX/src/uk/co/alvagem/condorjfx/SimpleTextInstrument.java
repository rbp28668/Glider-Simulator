/**
 * 
 */
package uk.co.alvagem.condorjfx;


import javafx.scene.Node;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.paint.Color;
import javafx.scene.paint.Paint;
import javafx.scene.text.Font;

import com.sun.javafx.tk.FontMetrics;
import com.sun.javafx.tk.Toolkit;

/**
 * @author bruce.porteous
 *
 */
public class SimpleTextInstrument extends RectangularInstrument {

	private Paint background = Color.LIGHTGREY;
	private Canvas canvas;
	/**
	 * @param width
	 * @param height
	 */
	SimpleTextInstrument(String key, int width, int height) {
		super(width, height);
		this.key = key;
	}

	protected String getValueAsKts() {
		String result = "------";
		if(this.value != null){
			double value = Double.parseDouble(this.value);
			value *= 1.94384449;  // ms to kts.
			result = String.format("%3.1f", value) + " kts";
		}
		return result;
	}

	protected String getValueAsFeet() {
		String result = "------";
		if(this.value != null){
			double value = Double.parseDouble(this.value);
			value *= 3.2808399;  // m to feet.
			result = String.format("%5.1f", value) + " feet";
		}
		return result;
	}

	protected String getValueAsRawFeet() {
		String result = "------";
		if(this.value != null){
			double value = Double.parseDouble(this.value);
			result = String.format("%5.1f", value) + " feet";
		}
		return result;
	}

	protected String getValueAsVarioKts() {
		String result = "------";
		if(this.value != null){
			double value = Double.parseDouble(this.value);
			boolean isUp = value > 0.0;
			value = Math.abs(value);
			value *= 1.94384449;  // ms to kts.
			result = String.format("%3.1f", value) + " kts ";
			if(value > 0.1) {
				result += (isUp) ? "UP" : "DOWN";
			}
			
		}
		return result;
	}
	
	protected String getValueAsDegrees() {
		String result = "------";
		if(this.value != null){
			double value = Double.parseDouble(this.value);
			result = String.format("%3.1f", value) + " degrees";
		}
		return result;
	}
	
	protected String getValueAsDegreesPerSec() {
		String result = "------";
		if(this.value != null){
			double value = Double.parseDouble(this.value);
			value *= 180.0 / Math.PI;  // radians per sec to degrees per sec.
			result = String.format("%3.1f", value) + " degrees/sec";
		}
		return result;
	}

	protected String getValueAsFrequency() {
		String result = "------";
		if(this.value != null){
			result = this.value + " MHz";
		}
		return result;
	}


	private void draw(GraphicsContext gc) {

		gc.setFill(background);
		gc.setStroke(background);
		gc.fillRect(0,0, width, height);

		gc.setStroke(Color.RED);
		gc.setFill(Color.RED);
		
		Font titleFont = new Font("SansSerif", 10);
		gc.setFont(titleFont);
		Toolkit tk = Toolkit.getToolkit();
		FontMetrics fm = tk.getFontLoader().getFontMetrics(titleFont);
		float tHeight = fm.getLineHeight();
		gc.fillText(this.key, tHeight, tHeight);
		
		Font font = new Font("SansSerif",  20);
		gc.setFont(font);
		
		FontMetrics metrics = tk.getFontLoader().getFontMetrics(font);
		float h = metrics.getLineHeight();
		gc.fillText(getValue(), h/2, h+(height-h)/2);
	}

	
	/* (non-Javadoc)
	 * @see uk.co.alvagem.condorjfx.AbstractInstrument#updated()
	 */
	@Override
	public void updated() {
		draw(canvas.getGraphicsContext2D());
	}

	/* (non-Javadoc)
	 * @see uk.co.alvagem.condorjfx.AbstractInstrument#getInstrumentNode()
	 */
	@Override
	public Node getInstrumentNode() {
		canvas = new Canvas(width,height);
		draw(canvas.getGraphicsContext2D());
		return canvas;
	}

	protected String getValue() {
		if(this.value == null) {
			return "------";
		}
		return this.value;
	}
	
	public static class Airspeed extends SimpleTextInstrument{
		
		public Airspeed(int width, int height){
			super("airspeed",width, height);
		}
		
		protected String getValue() {
			return getValueAsKts();
		}
	}
	
	public static class Altimeter extends SimpleTextInstrument{
		
		public Altimeter(int width, int height){
			super("altitude",width, height);
		}
		
		protected String getValue() {
			return getValueAsRawFeet();
		}
	}

	public static class Vario extends SimpleTextInstrument{
		
		public Vario(int width, int height){
			super("vario",width, height);
		}
		
		protected String getValue() {
			return getValueAsVarioKts();
		}
	}

	public static class EVario extends SimpleTextInstrument{
		
		public EVario(int width, int height){
			super("evario",width, height);
		}
		
		protected String getValue() {
			return getValueAsVarioKts();
		}
	}

	public static class Compass extends SimpleTextInstrument{
		
		public Compass(int width, int height){
			super("compass",width, height);
		}
		
		protected String getValue() {
			return getValueAsDegrees();
		}
	}

	public static class Slipball extends SimpleTextInstrument{
		
		public Slipball(int width, int height){
			super("slipball",width, height);
		}
		
		protected String getValue() {
			return getValueAsDegrees();
		}
	}

	public static class Turnrate extends SimpleTextInstrument{
		
		public Turnrate(int width, int height){
			super("turnrate",width, height);
		}
		
		protected String getValue() {
			return getValueAsDegreesPerSec();
		}
	}

	public static class Radio extends SimpleTextInstrument{
		
		public Radio(int width, int height){
			super("radiofrequency",width, height);
		}
		
		protected String getValue() {
			return getValueAsFrequency();
		}
	}

	
}
