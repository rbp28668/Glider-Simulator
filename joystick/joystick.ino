/* Updated USB Joystick code for Condor
   Teensy becomes a USB joystick with 16 or 32 buttons and 6 axis input

   You must select Joystick from the "Tools > USB Type" menu

   Pushbuttons should be connected between the digital pins and ground.
   Potentiometers should be connected to analog inputs 0 to 5.

*/

#include <math.h>

// Digital Inputs
#define FIRST_DIGITAL_INPUT 2
#define RELEASE 2
#define GEAR_DOWN 3
#define GEAR_UP 4
#define SW2_DOWN 5
#define SW2_UP 6
#define SW3_DOWN 7
#define SW3_UP 8
#define RS_BUTTON1 9
#define RS_BUTTON2 10
#define RS_BUTTON3 11
#define RS_BUTTON4 12
#define LAST_DIGITAL_INPUT 12

// Digital outputs
#define LED 13

// Analogue Inputs
#define AILERON 0
#define ELEVATOR 1
#define RUDDER 2
#define AIRBRAKE 3
#define KNOB2 6
#define THROTTLE 7



// Configure the number of buttons.  Be careful not
// to use a pin for both a digital button and analog
// axis.  The pullup resistor will interfere with
// the analog voltage.
const int numButtons = 14;  // up to 32, only bother with 14

const double pi = 3.14159265358979;

void setup() {
  // you can print to the serial monitor while the joystick is active!
  Serial.begin(9600);
  // configure the joystick to manual send mode.  This gives precise
  // control over when the computer receives updates, but it does
  // require you to manually call Joystick.send_now().
  Joystick.useManualSend(true);
  for (int i=FIRST_DIGITAL_INPUT; i<= LAST_DIGITAL_INPUT; i++) {
    pinMode(i, INPUT_PULLUP);
  }
  
  pinMode(LED, OUTPUT);  // LED on Teensy 3.1
  
 }


int count = 0;

void loop() {
  
  int aileron = analogRead(AILERON);
  int elevator = analogRead(ELEVATOR);
  int rudder = analogRead(RUDDER);
  int airbrake = analogRead(AIRBRAKE);
  int spareKnob = analogRead(KNOB2);
  int throttle = analogRead(THROTTLE);

  throttle /= 4;

  // aileron naturally reversed
  aileron = 1023 - aileron;

  const int divider = 2;
  
  // Signal conditioning.  
  aileron /= divider;
  elevator /= divider;
  rudder /= divider;
  airbrake /= divider;
  throttle /= divider;
  spareKnob /= divider;
  
  
  // Send the 6 analogue values to joystick.
  Joystick.X(aileron);    // roll
  Joystick.Y(elevator);    // pitch
  Joystick.Z(airbrake);    // brake?
  Joystick.Zrotate(rudder);  // yaw
  Joystick.sliderLeft(throttle);  // throttle knob
  Joystick.sliderRight(spareKnob); // pot

  int button = 1;
  Joystick.button(button++, digitalRead(RELEASE) ? 0 : 1 );
  Joystick.button(button++, digitalRead(GEAR_DOWN) ? 0 : 1 );
  Joystick.button(button++, digitalRead(GEAR_UP) ? 0 : 1 );
  Joystick.button(button++, digitalRead(SW2_DOWN) ? 0 : 1 );
  Joystick.button(button++, digitalRead(SW2_UP) ? 0 : 1 );
  Joystick.button(button++, digitalRead(SW3_DOWN) ? 0 : 1 );

    
  
  // Because setup configured the Joystick manual send,
  // the computer does not see any of the changes yet.
  // This send_now() transmits everything all at once.
  Joystick.send_now();


  Serial.print(aileron);
  Serial.print(",");
  Serial.print(elevator);
  Serial.print(",");
  Serial.print(airbrake);
  Serial.print(",");
  Serial.print(rudder);
  Serial.println();

  
  // Flash LED
  ++count;
  if(count > 50){
    digitalWrite(13,HIGH);
  } else {
    digitalWrite(13,LOW);
  }
  if(count > 100) count = 0;
  
  
  // a brief delay, so this runs 50 times per second
  delay(20);
}



