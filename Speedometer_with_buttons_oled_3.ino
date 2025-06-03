/*
 *  Speedometer version DV oled and buttons
 */ original code by @rwissbaum9849 on youtube channel "HO Scale Rio Grande in the San Luis Valley"

 //========== Customization ==================================

#include <Button.h>

Button button1(2); // Connect your button between pin 2 and GND
Button button2(3); // Connect your button between pin 3 and GND

//#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define XPOS 0
#define YPOS 1
#define DELTAY 2

/*
 * PIN connection:
 * pin connection see: https://www.arduino.cc/en/Reference/Wire
 * for UNO: SDA to A4, SCL to A5 // and nano
  * VCC to 5V
 * GND to GND :-)
 */
// this is the Width and Height of Display which is 128 xy 32 
#define LOGO16_GLCD_HEIGHT 32
#define LOGO16_GLCD_WIDTH  128 

// scale definitions
#define O    48
#define S    64
#define HO   87.1
#define OO   76
#define TT   120
#define Nb   148 // N british Nb
#define Ne   160 // N european Ne
#define Z    220 

// define array for scale (ignore S and TT)
float scaleA[6] = {O,HO,Ne,OO,Nb,Z};
String scaleN[6] = {"O","HO","Ne","OO","Nb","Z"};
float scale; // not sure if needed but used in constant calc
int PC = 0; // starting value for scale array counter

// speed conversion factors
#define dMPH 
#define MPH 1.0
#define KPH 1.609344
float speedUnits = MPH;       // select MPH or KPH
String unitsString = "MPH";

#define INCHES  1
#define CM      2
float distance = 8.0;         // enter distance between sensors
int distUnits = INCHES;       // select INCHES or CM

//=========== End Customization ===============================

#define ST_READY      1
#define ST_DETECT_LB  2
#define ST_DETECT_RB  3
#define ST_CALC       4
#define ST_WAITING    5

int state;

// pin assignments
// * for UNO: SDA to A4, SCL to A5 // also nano
const unsigned leftLED = A2;
const unsigned rghtLED = A3;
const unsigned SW1 = 2;
const unsigned SW2 = 3;
const int ledPin = 13;    // the number of the LED pin for testing

// variables that will change:

//int buttonState = 0;  // variable for reading the pushbutton status
unsigned long timerStart;
unsigned long elapsedTime;

float speed;
float constant;
 
String scaleString = "";
String text;
String display_string;
String vString;
String stringOne, stringTwo ;



void setup() {
  Serial.begin(9600);

  pinMode(rghtLED,INPUT);
  pinMode(leftLED,INPUT);
  pinMode(SW1,INPUT_PULLUP);
  pinMode(SW2,INPUT_PULLUP);

  button1.begin();
  button2.begin();

  // inches cm switch
  if (distUnits==CM) distance = distance / 2.54;

  state = ST_READY;


   // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.display();
 
  delay(2000);
   // Clear the buffer.
  display.clearDisplay(); 

//set some initial values 
PC = 0; // starting pointer for scale array
speed = 0;
constant = 0; // scale constant
scale = scaleA[0];
vString = "0.0";


  // up the inital display
oled_display2(speed, unitsString, scale, "Ready");
 
}

void loop() {


int newState=state;

// check buttons

if (button1.pressed()) { 
        
        if (speedUnits == MPH) {
            speedUnits = KPH;
            unitsString = "KPH";
            speed = speed * KPH;
            }
        else {
            speedUnits = MPH;
            unitsString = "MPH";  
            speed = speed / KPH;
             }
        
              // what if the arguments passed would be speed=000 , units, scaleString, status
       // state could be set back to ready
       //speed = 0; // consider a converted speed here instead of zero


       oled_display2(speed, unitsString, scale, "Ready");
           
    }
        

    if (button2.pressed()) {
        // increment press counter start at -1 then increment on press
        if (PC >= 5) (PC = -1); // zero start for array 0-5
        PC = ++PC ; // will incr to 0 from -1
        // debug on monitor
        
       // what if scale string were the whole description text eg "HO 1/87.1" then pass this to the diplay function. 
       // the arguments passed would be speed=000 , units, scaleString, status
       // state could be set back to ready

        scaleString = String(scaleA[PC],1);
        scale = scaleA[PC];
        speed = 0; // reset old speed since the old value is now wrong
        oled_display2(speed, unitsString, scale, "Ready");
      
    }

// calculate speed contant here?


  switch (state) {
    case ST_READY:

      if (detected(leftLED)) {
        timerStart = millis();
        newState = ST_DETECT_RB;
       oled_display2(speed, unitsString, scale, "Detected"); // dont change old speed yet
      }
      else if (detected(rghtLED)) {
        timerStart = millis();
        newState = ST_DETECT_LB;
         oled_display2(speed, unitsString, scale, "Detected");
      }
      break;

    case ST_DETECT_LB:
      if (detected(leftLED)) {
        elapsedTime = millis() - timerStart;
        newState = ST_CALC;
      }
      break;

    case ST_DETECT_RB:
      if (detected(rghtLED)) {
        elapsedTime = millis() - timerStart;
        newState = ST_CALC;
      }
      break;

    case ST_CALC:
      {
      oled_display2(speed, unitsString, scale, "Calcing");

      constant = distance * scale / 12. / 5280. * speedUnits;
      speed = constant / (elapsedTime / 1000. / 3600.); 

      newState = ST_WAITING;
      
      oled_display2(speed, unitsString, scale, "Waiting");

      timerStart = millis();
      break;
      }
  
    case ST_WAITING:
      // force delay of 2 seconds
      if (millis()-timerStart<2000) return;

      if (!detected(leftLED) && !detected(rghtLED)) {
        newState = ST_READY;
        oled_display2(speed, unitsString, scale, "Ready");
        
      }
      break;
}
  state = newState;
} // end loop



// user functions
/*
 * oled_text based on robojaxText(String text, int x, int y,int size, boolean d)
 / textcolor and size moved to setup
 * text is the text string to be printed
 * x is the integer x position of text
 * y is the integer y position of text
 * z is the text size, 1, 2, 3 etc
 * d is either "true" or "false". Not sure, use true
 */
void oled_Text(String text, int x, int y,int size, boolean d) {

  display.setTextSize(size); //could go in setup
  display.setTextColor(WHITE); // could go in setup
  display.setCursor(x,y);
  display.println(text);
  if(d){
    display.display();
  }
}

// used in sensor pins
boolean detected(unsigned pin) {
  return (analogRead(pin)<500);
}


// rethinked display
// every time we send to the display update all elements
// create a function that accepts 4 arguments
// speed, unitsString, scale, status
// convert values to strings // speed, scale

void oled_display2(float speed, String unitsString, float scale, String text) {

  display.clearDisplay();
  oled_Text("SPEED:          ", 4, 3, 1, false);
  vString =  String(speed, 1);// converted float speed calculation and one decimal place

  oled_Text(vString, 55, 3, 1, false);
  oled_Text(unitsString, 90, 3, 1, false); // unitsString is global and set using button press 
  
  scaleString = String(scaleA[PC],0); // display scale with no desimal places
  oled_Text(scaleN[PC]+" 1/"+scaleString, 3, 23, 1, false);
  oled_Text(text,80,23, 1, false);
  display.display();

}

