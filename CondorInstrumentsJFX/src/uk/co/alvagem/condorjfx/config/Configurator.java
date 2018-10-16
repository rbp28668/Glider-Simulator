/**
 * 
 */
package uk.co.alvagem.condorjfx.config;

import java.io.File;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.math.BigInteger;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBElement;
import javax.xml.bind.Marshaller;
import javax.xml.bind.Unmarshaller;
import javax.xml.transform.stream.StreamSource;

import uk.co.alvagem.condorjfx.Instrument;
import uk.co.alvagem.condorjfx.InstrumentLocation;
import uk.co.alvagem.condorjfx.Panel;
import uk.co.alvagem.condorjfx.config.generated.ObjectFactory;
import uk.co.alvagem.condorjfx.config.generated.PanelConfiguration;

/**
 * Loads a set of panels from an XML configuration file
 * @author bruce.porteous
 *
 */
public class Configurator {

	private static Map<String,Converter> primitiveClasses = new HashMap<String,Converter>();
	private static Map<String,String> primitiveNames = new HashMap<String,String>();
	
	static {
		primitiveClasses.put("string", new ConvertString());
		primitiveClasses.put("byte", new ConvertByte());
		primitiveClasses.put("short",new ConvertShort());
		primitiveClasses.put("int", new ConvertInt());
		primitiveClasses.put("long", new ConvertLong());
		primitiveClasses.put("float", new ConvertFloat());
		primitiveClasses.put("double", new ConvertDouble());
		primitiveClasses.put("boolean", new ConvertBoolean());
		
		// Invert the map so we can look up simple type by target class name.
		for(Map.Entry<String, Converter> e : primitiveClasses.entrySet() ){
			primitiveNames.put(e.getValue().getTargetClass().getName(), e.getKey());
		}
	}
	
	
	
	public Panel[] loadPanels(String path) throws Exception{
		File file = new File(path);
		
		JAXBContext jaxbContext = JAXBContext.newInstance(PanelConfiguration.class.getPackage().getName());
 
		// Ok, so because of the xsd style JAXB won't add an XMLRootElement annotation.  Hence we have to
		// mess about a bit telling it that it's dealing with a PanelConfiguration.
		Unmarshaller jaxbUnmarshaller = jaxbContext.createUnmarshaller();
		JAXBElement<PanelConfiguration> root = jaxbUnmarshaller.unmarshal(new StreamSource(file), PanelConfiguration.class);
		PanelConfiguration config = root.getValue();

		List<Panel> panels = new LinkedList<Panel>();
		for(uk.co.alvagem.condorjfx.config.generated.Panel pconfig : config.getPanel()) {
			Panel panel = configPanel(pconfig);
			panels.add(panel);
		}

		return panels.toArray(new Panel[panels.size()]);
	}
	
	/**
	 * @param pconfig
	 * @return
	 */
	private Panel configPanel(uk.co.alvagem.condorjfx.config.generated.Panel pconfig) throws Exception{
		
		Panel panel = new Panel();
		panel.setName(pconfig.getName());

		for(uk.co.alvagem.condorjfx.config.generated.Instrument instrumentConfig : pconfig.getInstrument()){
			InstrumentLocation instrument = configure(instrumentConfig);
			panel.getInstruments().add(instrument);
		}
		return panel;
	}


	/**
	 * Configures an instrument location and adds it to the panel
	 * @param instruments
	 * @param instrumentConfig
	 * @throws Exception
	 */
	private InstrumentLocation configure( uk.co.alvagem.condorjfx.config.generated.Instrument instrumentConfig) throws Exception{
		
		// <Instrument class="uk.co.alvagem.condorjfx.SimpleTextInstrument.Altimeter" x="456" y="123">
		// 	<Param name="width" type="int">300</Param>
		//  <Param name="height" type="int"50"</Param>
		// </Instrument>
				
		String className = instrumentConfig.getClassName();
		int x = instrumentConfig.getX().intValue();
		int y = instrumentConfig.getY().intValue();
		
		Class<?> instrumentClass;
		try {
			instrumentClass = Class.forName(className);
		} catch (Exception e) {
			throw new IllegalArgumentException("Trying to create instrument " + className + " but it is not a known class or cannot be created");
		}
		
		if(!Instrument.class.isAssignableFrom(instrumentClass)) {
			throw new IllegalArgumentException(className + " exists but is not a valid instrument class");
		}
		
		// Collect up the params for initialising the instrument
		
		List<uk.co.alvagem.condorjfx.config.generated.Instrument.Param> params =  instrumentConfig.getParam();
		Class<?>[] parameterTypes = new Class<?>[params.size()];
		Object[] initargs = new Object[params.size()];
		int idx = 0;
		for(uk.co.alvagem.condorjfx.config.generated.Instrument.Param sub : params) {
			String typeName = sub.getType();
			
			Converter type = primitiveClasses.get(typeName);
			if(type == null) {
				throw new ConfigurationException("Don't know how to use type " + typeName);
			}
			
			parameterTypes[idx] = type.getTargetClass();
			initargs[idx] = type.convert(sub.getValue());
			++idx;
		}
		
		
		Constructor<?> constructor = instrumentClass.getConstructor(parameterTypes);
		Instrument instrument = (Instrument)constructor.newInstance(initargs);
	
		return new InstrumentLocation(instrument, x, y);
	}

	/**
	 * Saves all the panels to XML.
	 * @param path
	 * @param panels
	 * @throws Exception
	 */
	public void savePanels(String path, Panel[] panels) throws Exception{
		
		PanelConfiguration config = new PanelConfiguration();
		
		for(Panel panel : panels) {
			addPanel(config, panel);
		}
		
		File file = new File(path);
		JAXBContext jaxbContext = JAXBContext.newInstance(PanelConfiguration.class.getPackage().getName());
		Marshaller jaxbMarshaller = jaxbContext.createMarshaller();
		
 
		// output pretty printed
		jaxbMarshaller.setProperty(Marshaller.JAXB_FORMATTED_OUTPUT, true);
 
		ObjectFactory of = new ObjectFactory();
		JAXBElement<PanelConfiguration> root = of.createPanelConfiguration(config);
		jaxbMarshaller.marshal(root, file);

	}

	/**
	 * Sets up a panel in the configuration
	 * @param config
	 * @param panel
	 */
	private void addPanel(PanelConfiguration config, Panel panel) {
		uk.co.alvagem.condorjfx.config.generated.Panel panelConfig = new uk.co.alvagem.condorjfx.config.generated.Panel();
		panelConfig.setName(panel.getName());
		
		for(InstrumentLocation location : panel.getInstruments().getInstruments()){
			uk.co.alvagem.condorjfx.config.generated.Instrument instrumentConfig = new uk.co.alvagem.condorjfx.config.generated.Instrument();
			instrumentConfig.setClassName(location.getInstrument().getClass().getName());
			instrumentConfig.setX(BigInteger.valueOf( (long)(0.5 + location.getX())));
			instrumentConfig.setY(BigInteger.valueOf( (long)(0.5 + location.getY())));
			
			addParameters(instrumentConfig, location.getInstrument());
			
			panelConfig.getInstrument().add(instrumentConfig);
		}
		
		config.getPanel().add(panelConfig);
	}

	
	/**
	 * Adds any parameters to the instrument configuration.
	 * @param instrumentConfig
	 * @param instrument
	 */
	private void addParameters(
			uk.co.alvagem.condorjfx.config.generated.Instrument instrumentConfig,
			Instrument instrument) {
		
		for(String paramName : instrument.getConfigFields()) {
			
			String getterName = "get" + Character.toUpperCase(paramName.charAt(0)) + paramName.substring(1);
			
			try {
				Method getter = instrument.getClass().getMethod(getterName);
				
				String targetClassName = getter.getReturnType().getName();
				String targetType = primitiveNames.get(targetClassName);
				
				Object value = getter.invoke(instrument);
				
				uk.co.alvagem.condorjfx.config.generated.Instrument.Param param = 
						new uk.co.alvagem.condorjfx.config.generated.Instrument.Param();
				
				param.setName(paramName);
				param.setType(targetType);
				param.setValue(value.toString());
				instrumentConfig.getParam().add(param);
				
			} catch (NoSuchMethodException | SecurityException
					| IllegalAccessException | IllegalArgumentException
					| InvocationTargetException e) {
				e.printStackTrace();
			}
		}
	}


	/**
	 * Provide type of, and conversion from string to object for a primitive type.
	 * @author bruce.porteous
	 *
	 */
	private static interface Converter {
		Class<?> getTargetClass();
		Object convert(String value);
	}
	
	private static class ConvertString implements Converter {

		/* (non-Javadoc)
		 * @see uk.co.alvagem.condorjfx.config.Configurator.Converter#getTargetClass()
		 */
		@Override
		public Class<?> getTargetClass() {
			return String.class;
		}

		/* (non-Javadoc)
		 * @see uk.co.alvagem.condorjfx.config.Configurator.Converter#convert(java.lang.String)
		 */
		@Override
		public Object convert(String value) {
			return new String(value);
		}
	}
	
	
	private static class ConvertByte implements Converter {

		/* (non-Javadoc)
		 * @see uk.co.alvagem.condorjfx.config.Configurator.Converter#getTargetClass()
		 */
		@Override
		public Class<?> getTargetClass() {
			return byte.class;
		}

		/* (non-Javadoc)
		 * @see uk.co.alvagem.condorjfx.config.Configurator.Converter#convert(java.lang.String)
		 */
		@Override
		public Object convert(String value) {
			return Byte.parseByte(value);
		}
		
	}
	private static class ConvertShort implements Converter {

		/* (non-Javadoc)
		 * @see uk.co.alvagem.condorjfx.config.Configurator.Converter#getTargetClass()
		 */
		@Override
		public Class<?> getTargetClass() {
			return short.class;
		}

		/* (non-Javadoc)
		 * @see uk.co.alvagem.condorjfx.config.Configurator.Converter#convert(java.lang.String)
		 */
		@Override
		public Object convert(String value) {
			return Short.parseShort(value);
		}
	}
	
	private static class ConvertInt implements Converter {

		/* (non-Javadoc)
		 * @see uk.co.alvagem.condorjfx.config.Configurator.Converter#getTargetClass()
		 */
		@Override
		public Class<?> getTargetClass() {
			return int.class;
		}

		/* (non-Javadoc)
		 * @see uk.co.alvagem.condorjfx.config.Configurator.Converter#convert(java.lang.String)
		 */
		@Override
		public Object convert(String value) {
			return Integer.parseInt(value);
		}
	}
	
	private static class ConvertLong implements Converter {

		/* (non-Javadoc)
		 * @see uk.co.alvagem.condorjfx.config.Configurator.Converter#getTargetClass()
		 */
		@Override
		public Class<?> getTargetClass() {
			return long.class;
		}

		/* (non-Javadoc)
		 * @see uk.co.alvagem.condorjfx.config.Configurator.Converter#convert(java.lang.String)
		 */
		@Override
		public Object convert(String value) {
			return Long.parseLong(value);
		}
	}

	private static class ConvertFloat implements Converter {

		/* (non-Javadoc)
		 * @see uk.co.alvagem.condorjfx.config.Configurator.Converter#getTargetClass()
		 */
		@Override
		public Class<?> getTargetClass() {
			return float.class;
		}

		/* (non-Javadoc)
		 * @see uk.co.alvagem.condorjfx.config.Configurator.Converter#convert(java.lang.String)
		 */
		@Override
		public Object convert(String value) {
			return Float.parseFloat(value);
		}
	}

	private static class ConvertDouble implements Converter {

		/* (non-Javadoc)
		 * @see uk.co.alvagem.condorjfx.config.Configurator.Converter#getTargetClass()
		 */
		@Override
		public Class<?> getTargetClass() {
			return double.class;
		}

		/* (non-Javadoc)
		 * @see uk.co.alvagem.condorjfx.config.Configurator.Converter#convert(java.lang.String)
		 */
		@Override
		public Object convert(String value) {
			return Double.parseDouble(value);
		}
	}

	private static class ConvertBoolean implements Converter {

		/* (non-Javadoc)
		 * @see uk.co.alvagem.condorjfx.config.Configurator.Converter#getTargetClass()
		 */
		@Override
		public Class<?> getTargetClass() {
			return boolean.class;
		}

		/* (non-Javadoc)
		 * @see uk.co.alvagem.condorjfx.config.Configurator.Converter#convert(java.lang.String)
		 */
		@Override
		public Object convert(String value) {
			return Boolean.parseBoolean(value);
		}
	}

	
	
}

