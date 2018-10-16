/**
 * 
 */
package uk.co.alvagem.condorjfx;

import javafx.scene.Node;
import javafx.scene.transform.Translate;

public class InstrumentLocation {
	Instrument instrument;
	private double initialX;
	private double initialY;
	private boolean isDragged = false;
	private Node node;
	private Translate translate;
	
	/**
	 * @param instrument
	 * @param x
	 * @param y
	 */
	public InstrumentLocation(Instrument instrument, int x, int y) {
		super();
		this.instrument = instrument;
		
		node =  instrument.getInstrumentNode();
		translate = new Translate(x,y);
		node.getTransforms().add(translate);
		setDragHandler();
	}
	
	void setDragHandler() {
		
		node.setOnMousePressed( me -> { 
			initialX = me.getX(); 
			initialY = me.getY();
			isDragged = false;
			} );
		
		node.setOnMouseDragged( me -> { 
			isDragged = true;
		});
		
		node.setOnMouseReleased( me -> {
			if(isDragged){
				double dx = me.getX() - initialX;
				double dy = me.getY() - initialY;
				
				translate.setX( translate.getTx() + dx);
				translate.setY( translate.getTy() + dy);
			}
			isDragged = false;
		});
	}

	/**
	 * @return the instrument
	 */
	public Instrument getInstrument() {
		return instrument;
	}
	/**
	 * @return the x
	 */
	public double getX() {
		return translate.getTx();
	}
	/**
	 * @return the y
	 */
	public double getY() {
		return translate.getTy();
	}
	
	/**
	 * Gets the JavaFX drawing node for this instrument.
	 * @return
	 */
	public Node getNode() {
		return node;
	} 
	
	
}