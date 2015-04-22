//Arduino Audio Visualizer using the SparkFun Spectrum Shield
//Aaron Christensen 2015
//with acknowledgement to Ben Moyes

//GLOBAL SET UPS
//For spectrum analyzer shield pins
const int spectrumReset = 5;
const int spectrumStrobe = 4;
int spectrumAnalog = 0;  //0 for left channel, 1 for right.

//Dotstar
int head = 0;
int brightness = 0;

//Timing
unsigned long currentMicros = 0;
unsigned long lastRead = 0; //Last time spectrum was read; long because will be operated with currentMicros

//Spectrum read values will be kept here; array will be used to compare real-time band intensity against previous.
int Spectrum[7];
int history[7][2] = {
  {
    0,0  }
  , //Row index 0 band 0
  {
    1,0  }
  , //Row index 1
  {
    2,0  }
  , //Row index 2
  {
    3,0  }
  , //Row index 3
  {
    4,0  }
  , //Row index 4
  {
    5,0  }
  , //Row index 5
  {
    6,0  }  //Row index 6
};

//Operator initialization
byte band;
int downScale = 80; //For scaling band reading from 10 bit to 0-5
int average = 0;
int sum = 0;
int i = 0;
float prop[7] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
float numerator = 0.0;


#include <Adafruit_DotStar.h>
// Because conditional #includes don't work w/Arduino sketches...
#include <SPI.h>         // COMMENT OUT THIS LINE FOR GEMMA OR TRINKET
//#include <avr/power.h> // ENABLE THIS LINE FOR GEMMA OR TRINKET

#define NUMPIXELS 120 // Number of LEDs in strip

// Here's how to control the LEDs from any two pins:
#define DATAPIN    11
#define CLOCKPIN   13
Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS);//, DATAPIN, CLOCKPIN);

// Hardware SPI is a little faster, but must be wired to specific pins
// (Arduino Uno = pin 11 for data, 13 for clock, other boards are different).
//Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS);

void setup() {
  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
  clock_prescale_set(clock_div_1); // Enable 16 MHz on Trinket
#endif

  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off ASAP

  startMonitor(); //Begin printing to serial monitor for debugging

  //Setup pins to drive the spectrum analyzer. 
  pinMode(spectrumReset, OUTPUT);
  pinMode(spectrumStrobe, OUTPUT);


  reset(); //Init spectrum analyzer
  ledFlash(); // Turn all LEDs on and off
}

void loop() {
  currentMicros = micros(); //Start micros, overflows and resets to 0 every 70 minutes

  if (currentMicros - lastRead >= 100000){ //Read ten times/second
    read(); //Reads and updates leds
  }
}

void reset(){
  digitalWrite(spectrumStrobe,LOW);
  //delay(1);
  digitalWrite(spectrumReset,HIGH);
  //delay(1);
  digitalWrite(spectrumStrobe,HIGH);
  //delay(1);
  digitalWrite(spectrumStrobe,LOW);
  //delay(1);
  digitalWrite(spectrumReset,LOW);
  //delay(1);
  // Reading the analyzer now will read the lowest frequency.
}

void cycle(){
  //Cycle analyzer
  digitalWrite(spectrumStrobe,HIGH);
  digitalWrite(spectrumStrobe,LOW);  
}

void read(){
  lastRead = currentMicros;
  reset();
  for(band=0; band <7; band++){   
    history[band][1] = history[band][0];
    history[band][0] = ((analogRead(0) + analogRead(1) ) / 2); //Read each channel and average
    if (history[band][0] < 100){
      history[band][0] = 0;
    }
    printBand(band);
    
    /*
    if (history[band][0] > history[band][1]){
      prop[band] += 0.3;
    }
    else if (history[band][0] == history[band][1]){
    }
    else{
      prop[band] -= 0.6;
    }
  */
    prop[band] = history[band][0]/1000;
    //Min/max - don't overdrive pins
    if (prop[band] > 1){
      prop[band] = 1;
    }
    else if (prop[band] < 0.05){
      prop[band] = 0.05;
    }
    
    //Finally, update
    updateLeds(band, prop[band]);
    cycle();
  }
  Serial.println(); 
}

void startMonitor(){
  Serial.begin(9600);
  //Serial.write("Monitor started.");
  //Serial.println();
}

void ledFlash(){

}

void printBand(int band){
  Serial.print(history[band][0]);
  Serial.print(",");
}

void updateLeds(int band, float prop){
  head = band * 17;
  brightness = 100*prop;
  for (i = head; i < head + 16; i++){
    strip.setPixelColor(i, 0, brightness, 0);
  }
  strip.show();
}


