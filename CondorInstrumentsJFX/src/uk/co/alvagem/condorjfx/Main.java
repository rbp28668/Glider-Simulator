/**
 * 
 */
package uk.co.alvagem.condorjfx;

import java.io.IOException;
import java.net.SocketException;
import java.util.HashMap;
import java.util.Map;
import java.util.Stack;

import javafx.application.Application;
import javafx.scene.Group;
import javafx.scene.Scene;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.paint.Color;
import javafx.stage.Stage;
import uk.co.alvagem.condorjfx.config.Configurator;
import uk.co.alvagem.condorjfx.k21.K21ASIWinter;
import uk.co.alvagem.condorjfx.k21.K21Altimeter;
import uk.co.alvagem.condorjfx.k21.K21VarioElectric;
import uk.co.alvagem.condorjfx.k21.K21VarioWinter;

/**
 * @author bruce.porteous
 *
 */
public class Main extends Application {

	private int port = 55278;
	private Panel[] panels = null;
	private Map<String,Panel> panelLookup = new HashMap<>();
	private Stack<Panel> panelStack = new Stack<>();
	private Panel panel = null;
	private boolean showControl = false;
	private boolean showDiagnostics = false;
	private boolean enableDrag = false;
	private int screenX = 0;
	private int screenY = 0; 
	private Stage primaryStage = null;
	private Commands commands = new Commands(); // Application wide command handler.
	private PacketReceiver receiver;
	private TestDialog testDialog = null;
	private static Main app;
	
	@Override
	public void start(Stage primaryStage) {
		this.primaryStage = primaryStage;
		app = this;
		
		try {

			commands.add(new SelectPanelCommand());

			startReceiver();

			primaryStage.setTitle("Condor Instruments JavaFX");
			primaryStage.setFullScreen(true);

			panel = loadPanels();  // returns the default panel.
			showPanel( panel);

			if(showControl){
				testDialog = new TestDialog(primaryStage, port);
			}
		}
		catch(Exception e) {
			System.err.println("Unable to start " + e.getMessage());
		}
	}

	public static Main getApp() {
		return app;
	}
	
	public Panel[] getPanels() {
		return panels;
	}
	
	/**
	 * @param panel
	 */
	private void showPanel(Panel panel) {
		Instruments instruments = panel.getInstruments();
		Group root = instruments.getInstrumentGroup();
		if(showDiagnostics) {
			addDiagnostics(root);
		}

		if(enableDrag) {
			for(InstrumentLocation il : instruments.getInstruments()) {
				il.enableDrag(true);
			}
		}
		
		Scene scene = new Scene(root, primaryStage.getWidth(), primaryStage.getHeight(), Color.rgb(20,20,20));

		// add the single node onto the scene graph
		primaryStage.hide();
		primaryStage.setScene(scene);
		primaryStage.show();
		receiver.setInstruments(instruments);
		if(testDialog != null) {
			testDialog.show(); // will have been hidden when primary stage hidden.
		}
	}


	/**
	 * Starts up the panel with the current settings.
	 * @throws SocketException
	 */
	private void startReceiver() throws SocketException {
		receiver = new PacketReceiver(commands, port);
		Thread thread = new Thread(receiver,"Condor Instruments JFX");
		thread.setDaemon(true); // so app can close with this running.
		thread.start();
	}

	/**
	 * @param root
	 */
	private void addDiagnostics(Group root) {
		Canvas c = new Canvas(1920,1200);
		GraphicsContext gc = c.getGraphicsContext2D();
		gc.setFill(Color.DARKGRAY);
		if(screenX > 0 && screenY > 0) {
			gc.fillRect(0, 0, screenX, screenY);
		} else {
			gc.fillRect(0, 0, 1280, 1024);
			gc.setFill(Color.LIGHTGRAY);
			gc.fillRect(0, 0, 1024, 768);
		}
		panel.getInstruments().drawDiagnostic(gc);
		root.getChildren().add(0,c);
	}


	private Panel loadPanels() throws Exception{

		Parameters params = getParameters();
		Map<String,String>argProps = params.getNamed();
		Panel panel = null;

		String panelName = argProps.get("panel");
		if(panelName == null) {
			panel = createTestPanel();
		} else {

			Configurator config = new Configurator();
			panels = config.loadPanels(panelName);
			if(panels.length == 0) {
				System.err.println("No panels in config file " + panelName);
				panel = createTestPanel();
			}

			indexPanels();

			if(panels.length == 1) {
				panel = panels[0];
			} else {
				String use = argProps.get("use");
				int idx = 0;
				if(use == null) {
					System.err.println("Multiple panels in config but no --use option given. Defaulting to 0");
				} else {
					idx = Integer.parseInt(use);
					if(idx < 0 || idx >= panels.length) {
						System.err.println("Parameter for --use is out of range of panels supplied in config, defaulting to 0");
						idx = 0;
					}
				}
				panel = panels[idx];
			}
		}

		String portStr = argProps.get("port");
		if(portStr != null) {
			port = Integer.parseInt(portStr);
		}


		showControl =  "true".equals(argProps.get("dialog"));
		showDiagnostics =  "true".equals(argProps.get("diagnostics"));
		if(showDiagnostics) {
			String str = argProps.get("screenx");
			if(str != null) {
				screenX = Integer.parseInt(str);
			}
			str = argProps.get("screeny");
			if(str != null) {
				screenY = Integer.parseInt(str);
			}
		}
		enableDrag =  "true".equals(argProps.get("drag"));
		return panel;
	}


	/**
	 * Indexes the panels array so that we can access them by name.
	 */
	private void indexPanels() {
		for(Panel p: panels) {
			panelLookup.put(p.getName(),p);
		}
	}

	/**
	 * Selects a panel by name.
	 * @param name is the panel name to lookup the panel to display.
	 */
	public void selectPanel(String name) {
		Panel p  = panelLookup.get(name);
		if(p != null) {
			showPanel(p);
		}
	}

	/**
	 * Pushes the existing panel on a stack and selects a new one to display.
	 * @param name is the name of the new panel to display.
	 */
	public void pushPanel(String name) {
		panelStack.push(panel);
		selectPanel(name);
	}

	/**
	 * Pops any panel from the panel stack and displays it.
	 */
	public void popPanel() {
		if(!panelStack.isEmpty()) {
			showPanel(panelStack.pop());
		}
	}
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		launch(args);
	}


	/**
	 * @return
	 * @throws IOException
	 */
	protected static Panel createTestPanel() throws Exception {
		Panel panel = new Panel();
		panel.setName("Test Panel");
		int y = 0;
		final int LEFT = 100;
		final int SPACING = 70;

		panel.getInstruments().add(new SimpleTextInstrument.Airspeed( 200,50), LEFT, 100 + SPACING * y++ );
		panel.getInstruments().add(new SimpleTextInstrument.Altimeter(200,50), LEFT, 100 + SPACING * y++);
		panel.getInstruments().add(new SimpleTextInstrument.Vario(200,50), LEFT,100 + SPACING * y++);
		panel.getInstruments().add(new SimpleTextInstrument.EVario(200,50),LEFT,100 + SPACING * y++);
		panel.getInstruments().add(new SimpleTextInstrument.Compass(200,50),LEFT,100 + SPACING * y++);
		panel.getInstruments().add(new SimpleTextInstrument.Slipball(200,50), LEFT,100 + SPACING * y++);
		panel.getInstruments().add(new SimpleTextInstrument.Turnrate(200,50), LEFT,100 + SPACING * y++);
		panel.getInstruments().add(new SimpleTextInstrument.Radio(200,50), LEFT,100 + SPACING * y++);

		panel.getInstruments().add(new K21ASIWinter(300) ,500,100 );
		panel.getInstruments().add(new K21Altimeter(300) ,850,100 );
		panel.getInstruments().add(new K21VarioWinter(300), 500,450);
		panel.getInstruments().add(new K21VarioElectric(300), 850,450);
		return panel;
	}

	private class SelectPanelCommand extends Command {

		public SelectPanelCommand(){
			super("Panel");
		}

		/* (non-Javadoc)
		 * @see uk.co.alvagem.condorjfx.Command#execute()
		 */
		@Override
		public void execute(String cmd) {
			String parts[] = cmd.split("\\b");

			if(parts.length > 2) {
				int index = Integer.parseInt(parts[2]);
				if(index < panels.length && index >= 0) {
					Panel panel = panels[index];
					showPanel(panel);
				} else {
					System.err.println("Index of panels in out of range");
				}
			}

		}

	}
}
