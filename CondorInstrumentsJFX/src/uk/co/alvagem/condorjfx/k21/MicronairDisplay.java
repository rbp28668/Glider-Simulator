package uk.co.alvagem.condorjfx.k21;

import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.paint.Color;

/*5x7 matrix

Left 49 pixels
Right 587
Assume 1 pixel between chars

7 chars, less 1 pixel - 41 pixels -> 538 pixels

Each radio pixel takes up 13.12 pixels - say 13 pixels.

*/

// Area on image 244,447 -> 934,746  690 x 299



class MicronairDisplay  {
	
	private final static Color PIXEL_COLOUR = Color.rgb(0x24,0x2c,0x2f);
	private final static Color PIXEL_EDGE_COLOUR = Color.rgb(0x69, 0x76, 0x6d);
	private final static Color BACK_COLOUR = Color.rgb(0x59,0x8a,0x4e);
	
	private final static int PIXEL_WIDTH = 13;
	private final static int PIXEL_HEIGHT = 13;
	
	private final static int LEFT_MARGIN = 48; 
	private final static int LINE1_BASE = 18;
	private final static int LINE2_BASE = 151;

	private Canvas c;
	
	private static final byte[] BITMAP = {
	
	//0
	(byte)0b00001110,
	(byte)0b00010001,
	(byte)0b00010011,
	(byte)0b00010101,
	(byte)0b00011001,
	(byte)0b00010001,
	(byte)0b00001110,
	
	//1
	(byte)0b00000100,
	(byte)0b00001100,
	(byte)0b00000100,
	(byte)0b00000100,
	(byte)0b00000100,
	(byte)0b00000100,
	(byte)0b00001110,
	
	//2
	(byte)0b00001110,
	(byte)0b00010001,
	(byte)0b00000001,
	(byte)0b00000010,
	(byte)0b00000100,
	(byte)0b00001000,
	(byte)0b00011111,
	
	//3
	(byte)0b00011111,
	(byte)0b00000010,
	(byte)0b00000100,
	(byte)0b00000010,
	(byte)0b00000001,
	(byte)0b00010001,
	(byte)0b00001110,
	
	//4
	(byte)0b00000010,
	(byte)0b00000110,
	(byte)0b00001010,
	(byte)0b00010010,
	(byte)0b00011111,
	(byte)0b00000010,
	(byte)0b00000010,
	
	//5
	(byte)0b00011111,
	(byte)0b00010000,
	(byte)0b00011110,
	(byte)0b00000001,
	(byte)0b00000001,
	(byte)0b00010001,
	(byte)0b00001110,
	
	//6
	(byte)0b00000110,
	(byte)0b00001000,
	(byte)0b00010000,
	(byte)0b00011110,
	(byte)0b00010001,
	(byte)0b00010001,
	(byte)0b00001110,
	
	//7
	(byte)0b00011111,
	(byte)0b00000001,
	(byte)0b00000010,
	(byte)0b00000100,
	(byte)0b00001000,
	(byte)0b00001000,
	(byte)0b00001000,
	
	//8
	(byte)0b00001110,
	(byte)0b00010001,
	(byte)0b00010001,
	(byte)0b00001110,
	(byte)0b00010001,
	(byte)0b00010001,
	(byte)0b00001110,
	
	//9
	(byte)0b00001110,
	(byte)0b00010001,
	(byte)0b00010001,
	(byte)0b00001111,
	(byte)0b00000001,
	(byte)0b00000010,
	(byte)0b00001100,
	
	//.
	(byte)0b00000000,
	(byte)0b00000000,
	(byte)0b00000000,
	(byte)0b00000000,
	(byte)0b00000000,
	(byte)0b00001100,
	(byte)0b00001100,
	};

	Canvas createNode(String active, String standby) {
		c = new Canvas(690,300);
		update(active, standby);
		return c;
	}

	/**
	 * @param active
	 * @param standby
	 */
	void update(String active, String standby) {
		GraphicsContext gc = c.getGraphicsContext2D();
		gc.setFill(BACK_COLOUR);
		gc.fillRect(0, 0, c.getWidth(), c.getHeight());
		
		gc.setFill(PIXEL_COLOUR);
		gc.setStroke(PIXEL_EDGE_COLOUR);
		drawString(gc, active, LEFT_MARGIN, LINE1_BASE );
		drawString(gc, standby, LEFT_MARGIN, LINE2_BASE);
	}
	
	
	private void drawString(GraphicsContext gc, String text, int ix, int iy) {
		
		int len = text.length();
		if(len > 8) {
			len = 8;
		}
		
		
		for(int i=0; i<len; ++i) {
			char ch = text.charAt(i);
			
			int charStart = 0;
			switch(ch) {
			case '0': charStart = 0; break;
			case '1': charStart = 7; break;
			case '2': charStart = 14; break;
			case '3': charStart = 21; break;
			case '4': charStart = 28; break;
			case '5': charStart = 35; break;
			case '6': charStart = 42; break;
			case '7': charStart = 49; break;
			case '8': charStart = 56; break;
			case '9': charStart = 63; break;
			case '.': charStart = 70; break;
			}
			
			drawChar(gc, charStart, ix, iy);
			ix += 6 * PIXEL_WIDTH;
		}
	}
	
	private void drawChar(GraphicsContext gc, int charStart, int ix, int iy) {
		
		for(int row = 0; row < 7; ++row) {
			drawRow(gc, BITMAP[charStart + row], ix, iy);
			iy += PIXEL_HEIGHT;
		}
		
	}
	private void drawRow(GraphicsContext gc, byte b, int ix, int iy){
		
		byte mask = (byte)0b00010000;
		while(mask != 0) {
			if( (b & mask) != 0) {
				gc.fillRect(ix,iy, PIXEL_WIDTH,PIXEL_HEIGHT);
				gc.strokeRect(ix, iy, PIXEL_WIDTH,PIXEL_HEIGHT);
			}
			ix += PIXEL_WIDTH;
			mask >>=1;
		}
	}
}










