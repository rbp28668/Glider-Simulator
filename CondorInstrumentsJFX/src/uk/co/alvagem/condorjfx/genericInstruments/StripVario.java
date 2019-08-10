package uk.co.alvagem.condorjfx.genericInstruments;

import com.sun.javafx.tk.FontMetrics;
import com.sun.javafx.tk.Toolkit;

import javafx.scene.Node;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.paint.Color;
import javafx.scene.text.Font;
import uk.co.alvagem.condorjfx.AbstractInstrument;
import uk.co.alvagem.condorjfx.Instrument;

public class StripVario extends AbstractInstrument implements Instrument {

	private static final int MAX_ENDSTOP = 10;
	private static final int MIN_ENDSTOP = -10;
	private final static Color BACKGROUND_COLOUR = Color.rgb(43,43,45);
	private final static Color TEXT_COLOUR = Color.rgb(254,254,254);
	private Canvas canvas;

	private int height;

	public StripVario(int width, int height) {
		super(width);
		this.key = "vario";
		this.height = height;
	}

	
	public int getHeight() {
		return height;
	}


	private void draw(GraphicsContext gc) {

		gc.setFill(BACKGROUND_COLOUR);
		gc.setStroke(BACKGROUND_COLOUR);
		gc.fillRect(0,0, width, height);

		gc.setStroke(TEXT_COLOUR);
		gc.setFill(TEXT_COLOUR);
		gc.setLineWidth(1.0);
		
		double fontSize = height / 30; // font size of 20 about right for 600 high vario
		Font font = new Font("SansSerif", fontSize);
		gc.setFont(font);
		Toolkit tk = Toolkit.getToolkit();
		FontMetrics fm = tk.getFontLoader().getFontMetrics(font);
		float h = fm.getAscent();

		double dy = height / (1 + MAX_ENDSTOP - MIN_ENDSTOP);
		double y = dy/2; // start height for first tick.
		for(int i=MAX_ENDSTOP; i>=MIN_ENDSTOP; --i) {
			gc.strokeLine(0, y, width/4, y);
			String val = Integer.toString(i);
			gc.fillText(val, width/2, y+h/2);
			y += dy;
		}
		y = dy/2 + (MAX_ENDSTOP - getValue() ) * dy;
		double xPoints[] = {0.0,width/2,0.0};
		double yPoints[] = {y-width/4, y, y+width/4};
		//gc.strokePolygon(xPoints, yPoints, 3);
		gc.fillPolygon(xPoints, yPoints, 3);
		
	}


	@Override
	public void updated() {
		draw(canvas.getGraphicsContext2D());
	}

	@Override
	public Node getInstrumentNode() {
		canvas = new Canvas(width,height);
		draw(canvas.getGraphicsContext2D());
		return canvas;
	}


	@Override
	public String[] getConfigFields() {
		String[] fields =  { "width", "height" }; // must match constructor
		return fields;	}
	
	private double getValue() {
		double value = 0.0;
		if(this.value != null){
			value = Double.parseDouble(this.value);
			value *= 1.94384449;  // ms to kts.
			if(value > MAX_ENDSTOP) {
				value = MAX_ENDSTOP;
			} else if (value < MIN_ENDSTOP){
				value = MIN_ENDSTOP;
			}
		}
		return value;	
	}


}
