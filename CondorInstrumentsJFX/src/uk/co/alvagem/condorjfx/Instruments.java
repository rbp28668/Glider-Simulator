/**
 * 
 */
package uk.co.alvagem.condorjfx;

import java.util.LinkedList;
import java.util.List;

import javafx.scene.Group;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.transform.Affine;

/**
 * A collection of instruments that knows how to respond to a data packet and provide a JFX node for drawing.
 * @author bruce.porteous
 *
 */
public class Instruments {

	// time in-game day time decimal hours
	// airspeed airspeed indicator reading m/s
	// altitude altimeter reading m
	// vario pneumatic variometer reading m/s
	// evario electronic variometer reading m/s
	// nettovario netto variometer value m/s
	// integrator integrator value m/s
	// compass compass reading degrees
	// slipball slip ball deflection angle rad
	// turnrate turn indicator reading rad/s
	// yawstringangle yawstring angle rad
	// radiofrequency radio frequency mhz
	// yaw yaw rad
	// pitch pitch rad
	// bank bank rad

	private List<InstrumentLocation> instruments = new LinkedList<InstrumentLocation>();
	
	public void add(Instrument instrument, int x, int y) throws Exception {
		if(instrument == null) {
			throw new NullPointerException("Cannot add a null instrument");
		}
		instruments.add(new InstrumentLocation(instrument, x, y));
	}
	
	public void add(InstrumentLocation instrumentLocation) {
		instruments.add(instrumentLocation);
	}
	
	public void dispatch(Packet packet) {
		for(InstrumentLocation instrument : instruments){
			instrument.getInstrument().process(packet.getReadings());
		}
	}

	public InstrumentLocation[] getInstruments() {
		return instruments.toArray(new InstrumentLocation[instruments.size()]);
	}
	
	public void drawDiagnostic(GraphicsContext gc) {
		for(InstrumentLocation instrument : instruments){
			double x = instrument.getX();
			double y = instrument.getY();
			Affine originalTransform = gc.getTransform();
			gc.translate(x,y);
			instrument.getInstrument().drawDiagnostic(gc);
			gc.setTransform(originalTransform);
		}
		
	}

	public Group getInstrumentGroup(){
		Group group = new Group();
		for(InstrumentLocation instrument : instruments){
			group.getChildren().add(instrument.getNode());
		}
		
		return group;
	}
	

	
}
