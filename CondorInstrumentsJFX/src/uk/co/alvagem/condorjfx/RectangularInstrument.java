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
	
	protected RectangularInstrument(int width, int height) {
		super(width);
		this.width = width;
		this.height = height;
	}
	
	/* (non-Javadoc)
	 * @see uk.co.alvagem.condorjfx.Instrument#getConfigFields()
	 */
	@Override
	public String[] getConfigFields() {
		String[] fields =  { "width", "height"};
		return fields;
	}
	
	public int getHeight() {
		return height;
	}
	
}
