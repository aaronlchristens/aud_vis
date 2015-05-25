//Arduino Audio Visualizer using the SparkFun Spectrum Shield
//Aaron Christensen 2015
//with acknowledgement to Ben Moyes

#include <Streaming.h>
const int spectrumReset = 5;
const int spectrumStrobe = 4;
int spectrumAnalog = 0; //0 for left channel, 1 for right.
int head = 0; //Dotstar
unsigned long currentMicros = 0;
unsigned long lastRead = 0; //Last time spectrum was read; long because will be operated with currentMicros
byte band;
uint8_t volume[7] = { 0, 0, 0, 0, 0, 0, 0};

#include <Adafruit_DotStar.h>
// Because conditional #includes don't work w/Arduino sketches...
#include <SPI.h>         // COMMENT OUT THIS LINE FOR GEMMA OR TRINKET
//#include <avr/power.h> // ENABLE THIS LINE FOR GEMMA OR TRINKET

#define NUMPIXELS 120 // Number of LEDs in strip

// Here's how to control the LEDs from any two pins:
#define DATAPIN    11
#define CLOCKPIN   13
Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS);//, DATAPIN, CLOCKPIN);

int displayLength = int(NUMPIXELS / 7);
// Hardware SPI is a little faster, but must be wired to specific pins
// (Arduino Uno = pin 11 for data, 13 for clock, other boards are different).
//Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS);

void setup() {
  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
  //clock_prescale_set(clock_div_1); // Enable 16 MHz on Trinket
#endif
  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off ASAP
  startMonitor(); //Begin printing to serial monitor for debugging
  //Setup pins to drive the spectrum analyzer. 
  pinMode(spectrumReset, OUTPUT);
  pinMode(spectrumStrobe, OUTPUT);
  reset(); //Init spectrum analyzer
}

void loop() {
  currentMicros = micros(); //Start micros, overflows and resets to 0 every 70 minutes
  //fails after 70 minutes? --> no
  if (currentMicros - lastRead >= 100000){ //Read 10 times/second
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
  //delay(5);
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
    volume[band] = (((analogRead(0) + analogRead(1) ) / 2)/4); //Read each channel and average
    //analog reads in voltage 0-5V or 0-1023, so divide by four to scale to one byte (0-255)
    if (volume[band] < 20){
      volume[band] = 0;
    }
    //printBand(band);   
    updateLeds(band);
    cycle();
  }
  //Serial.println(); 
}

void startMonitor(){
  Serial.begin(115200);
  //Serial.write("Monitor started.");
  //Serial.println();
}

void printBand(int band){
  Serial.print(volume[band]);
  Serial.print(",");
}

void updateLeds(int band){
  head = band * displayLength;
  for (int i = head; i < head + (displayLength - 1); i++){
    switch(band){
      case 0:
        strip.setPixelColor(i, volume[band], 0,0);
        Serial << band << ": " << volume[band] << 0 << " " << 0 << endl;
      case 1:
        strip.setPixelColor(i, volume[band], 0, 0);
        Serial << band << ": " << volume[band] << " " << volume[band] << " 0" << endl;
      case 2:
        strip.setPixelColor(i, volume[band], volume[band], 0);
        Serial << band << ": " << volume[band] << " " << volume[band] << " 0" << endl;
      case 3:
        strip.setPixelColor(i, volume[band]/2, volume[band], 0);
        Serial << band << ": " << volume[band]/2 << " " << volume[band] << " 0" << endl;
      case 4:
        strip.setPixelColor(i, 0, volume[band], volume[band]);
        Serial << band << ": " << "0 " << volume[band] << " " << volume[band] << endl;
      case 5:
        strip.setPixelColor(i, 0,0, volume[band]);
        Serial << band << ": " << "0 " << "0 " << volume[band] << endl;
      case 6:
        strip.setPixelColor(i, volume[band], 0, volume[band]);
        Serial << band << ": " << volume[band] << " 0 " << volume[band] << endl;
    }
  }
  strip.show();
}


