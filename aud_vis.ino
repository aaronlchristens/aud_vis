/*
Arduino Audio Visualizer (using the SparkFun Spectrum Shield)
Aaron Christensen 2015
Acknowledgement to Ben Moyes for Spectrum Shield strobing code
*/

/*
***LIBRARIES***
*/
#include <Streaming.h> // Provides simple functions for printing to serial monitor
#include <Adafruit_DotStar.h>
// Because conditional #includes don't work w/Arduino sketches:
#include <SPI.h>         // COMMENT OUT THIS LINE FOR GEMMA OR TRINKET
//#include <avr/power.h> // ENABLE THIS LINE FOR GEMMA OR TRINKET

/*
***HARDWARE-SPECIFIC VARIABLES***
*/
#define NUMPIXELS 120 // Number of LEDs in strip
/* Here's how to control the LEDs from any two pins:
Hardware SPI is a little faster, but must be wired to specific pins
(Arduino Uno = pin 11 for data, 13 for clock, other boards are different).
*/
#define DATAPIN    11
#define CLOCKPIN   13
Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS);//, DATAPIN, CLOCKPIN);
const int spectrumReset = 5; // Fixed for the Sparkfun Spectrum Shield
const int spectrumStrobe = 4; // Fixed for the Sparkfun Spectrum Shield

/*
***SOFTWARE VARIABLES***
*/
// SPECTRUM SHIELD
int spectrumAnalog = 0; // 0 for left channel, 1 for right.

// TIME
unsigned long currentMicros = 0;
unsigned long lastRead = 0; // Last time spectrum was read; long because will be operated with currentMicros
// add a variable that allows for direct/simple manipulation of reads/second

// DATA STORAGE
byte band;
uint8_t volume[7] = { 0, 0, 0, 0, 0, 0, 0 };
// ^^^ make this 7x2?

// AUDIO
const uint8_t volumeMax = 0; // One bit: 1024 from each band (* 7) scaled down (/ (4 * 7))
uint8_t volumeTotal = 0; // Will hold sum of volumes from each band (sum of volume array)

// DISPLAY
byte mode = 0;
uint8_t displayLengthMax = 0;
uint8_t displayLength = 0; // Calculated display length
uint32_t displayColor = 0xFF0000; // Start red
uint8_t displayRed = 255; // Start red
uint8_t displayGreen = 0;
uint8_t displayBlue = 0;

// Code ran initially and once when the Arduino is started
void setup() {
  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
  //clock_prescale_set(clock_div_1); // Enable 16 MHz on Trinket
#endif
  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off
  pinMode(spectrumReset, OUTPUT); // Init shield pins
  pinMode(spectrumStrobe, OUTPUT);
  reset(); // Init spectrum analyzer

  // SERIAL MONITOR
  //startMonitor(); // Begin printing to serial monitor for debugging
}

// Code ran in a loop after setup
void loop() {
  currentMicros = micros(); // Start micros clock, overflows and resets to 0 every 70 minutes (fails after 70 minutes? --> no)
  if (currentMicros - lastRead >= 75000){ // Read 1000000/n times/second
    readAudio();
    updateLeds();
  }
}

// Reads audio via Spectrum Shield, stores in array
void readAudio(){
  reset();
  for(band = 0; band < 7; band++){
    volume[band] = (((analogRead(0) + analogRead(1) ) / 2)/28); // Read each channel and average
    // Analog reads in voltage 0-5V or 0-1023, so divide by 4*7 to scale to 1/7 of a byte
    if (volume[band] < 20){ // (Try to) filter out noise
      volume[band] = 0;
    }   
    cycle();

    // MONITOR
    //printBand(band);
  }
  lastRead = currentMicros;
  //Serial.println(); 
}

void updateLeds(){
  strip.clear();
  if (mode == 0){
      displayLengthMax = NUMPIXELS / 2; // Displaying from midpoint outward
      // Shift color on bass hits
      if (volume[2] > 150){
        /*  
        if ((displayColor >>= 8) == 0){ // Next color (R->G->B) ... past blue now?
              displayColor = 0xFF0000; // Yes, reset to red
          }
        */
        if (displayRed == 255){
          displayRed = 0;
          displayGreen = 255;
        }
        else if (displayGreen == 255){
          displayGreen = 0;
          displayBlue = 255;
        }
        else if (displayBlue == 255){
          displayBlue = 0;
          displayRed = 255;
        }
      }

      // Sum the volumes
      volumeTotal = 0;
      for (band = 0; band < 7; band++){
          volumeTotal += volume[band];
      }
      displayLength = (displayLengthMax * volumeTotal) / volumeMax; // Calculate how much to display based on volume
      for (int i = 61; i < NUMPIXELS; i++){ // Update second half of strip
          if (i < (displayLength + 60)){
              strip.setPixelColor(i, displayRed, displayGreen, displayBlue);
          }
          else{
              strip.setPixelColor(i, 10, 10, 10);
          }
      }
      for (int i = 60; i > -1; i--){ // Update first half of strip
          if (i > (60 - displayLength)){
              strip.setPixelColor(i, displayRed, displayGreen, displayBlue);
          }
          else{
              strip.setPixelColor(i, 10, 10, 10);
          }
      }
  }
  strip.show(); // Display
}

// Cycle analyzer to read the next highest band
void cycle(){
  digitalWrite(spectrumStrobe, HIGH);
  digitalWrite(spectrumStrobe, LOW);
}

void startMonitor(){
  Serial.begin(115200);
}

// Prints "volume[band],"
void printBand(int band){
  Serial.print(volume[band]);
  Serial.print(",");
}

// Run to have Spectrum Shield to read lowest frequency after
void reset(){
  digitalWrite(spectrumStrobe, LOW);
  //delay(1);
  digitalWrite(spectrumReset, HIGH);
  //delay(1);
  digitalWrite(spectrumStrobe, HIGH);
  //delay(1);
  digitalWrite(spectrumStrobe, LOW);
  //delay(1);
  digitalWrite(spectrumReset, LOW);
  //delay(5);
  // Reading the analyzer now will read the lowest frequency.
}
