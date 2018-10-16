/**
 * 
 */
package uk.co.alvagem.condorjfx;


/**
 * @author bruce.porteous
 *
 */
public class RectangularInstrument extends AbstractInstrument {

	protected int height;
	
	RectangularInstrument(int width, int height) {
		super(width);
		this.width = width;
		this.height = height;
	}
	
}
