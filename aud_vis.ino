//Arduino Audio Visualizer using the SparkFun Spectrum Shield
//Aaron Christensen 2015
//with acknowledgement to Ben Moyes, Lady Ada, Jack Christensen

#include <Streaming.h>
#include <Adafruit_DotStar.h>
// Because conditional #includes don't work w/Arduino sketches...
#include <SPI.h>         // COMMENT OUT THIS LINE FOR GEMMA OR TRINKET
//#include <avr/power.h> // ENABLE THIS LINE FOR GEMMA OR TRINKET

#define NUMPIXELS 120 // Number of LEDs in strip

// Here's how to control the LEDs from any two pins:
#define DATAPIN    11
#define CLOCKPIN   13
Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DATAPIN, CLOCKPIN);

const int spectrumReset = 5;
const int spectrumStrobe = 4;
int spectrumAnalog = 0; // 0 for left channel, 1 for right.
//int head = 0; // Dotstar
unsigned long currentMicros = 0;
unsigned long lastRead = 0; //Last time spectrum was read; long because will be operated with currentMicros
byte band;
uint32_t volume[7][2] = { {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} };
//const uint8_t volumeMax = 1023 / 4; //1024 from each band (* 7) scaled down (/ (4 * 7)) to be one bit
//const uint8_t displayLengthMax = strip.numPixels() / 2; //Displaying from midpoint outward

//uint8_t displayLength = 0;

void setup() {
  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
  //clock_prescale_set(clock_div_1); // Enable 16 MHz on Trinket
#endif
  strip.begin(); // Initialize pins for output
  
  for (int i = 0; i < NUMPIXELS; i++) {
	  strip.setPixelColor(i, i, 0, 0);
	  strip.show();
	  delay(10);
  }
  
  //startMonitor(); //Begin printing to serial monitor for debugging
  //Setup pins to drive the spectrum analyzer. 
  pinMode(spectrumReset, OUTPUT);
  pinMode(spectrumStrobe, OUTPUT);
  reset(); //Init spectrum analyzer
}

void loop() {
  currentMicros = micros(); //Start micros, overflows and resets to 0 every 70 minutes
  //fails after 70 minutes? --> no
  if (currentMicros - lastRead >= 50000){ 
    readAudio();
    updateLeds();
  }
}

void readAudio(){
  reset();

  for(band=0; band <7; band++){
	volume[band][1] = volume[band][0];
	volume[band][0] = ((analogRead(0) + analogRead(1) ) / 2); //Read each channel and average
	//if volume
    //analog reads in voltage 0-5V or 0-1023 bits (4 bytes), so divide by 4*7 to scale to 1/7 of a byte (7 bands)
    if (volume[band][0] < 100){
      volume[band][0] = 0;
    }
    cycle();
  }
  lastRead = currentMicros;
}

void updateLeds(){
  unsigned int volumeTotal = (volume[0][0] + volume[1][0]+ volume[2][0] + volume[3][0] + volume[4][0] + volume[5][0] + volume[6][0]);
  for (int i = 0; i < strip.numPixels(); i++) {
		strip.setPixelColor(i, Wheel3(calcFrac(volume[maxBand()][0],1023)));
	}
	strip.show(); 
}

void cycle(){
    //Cycle analyzer
    digitalWrite(spectrumStrobe, HIGH);
    digitalWrite(spectrumStrobe, LOW);
}

void startMonitor(){
  //Serial.begin(115200);
}

void printBand(int band){
  Serial.print(volume[band][0]);
  Serial.print(",");
}

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

// Wheel written by Lady Ada for NeoPixels, adjusted for Dotstars by Jack Christensen
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(uint8_t WheelPos)
{
	WheelPos = 255 - WheelPos;
	if (WheelPos < 85) {
		return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
	}
	else if (WheelPos < 170) {
		WheelPos -= 85;
		return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
	}
	else {
		WheelPos -= 170;
		return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
	}
}

uint32_t Wheel2(uint8_t WheelPos) {
	return strip.Color(WheelPos, 255, 0);
}

int calcFrac(double numer, double denom) {
	float volFrac = numer / denom;
	unsigned int wheelInput = 255 * volFrac;

	if (wheelInput > 255) {
		wheelInput = 255;
	}
	return(wheelInput);
}

byte maxBand() {
	byte maxBand = 0;
	for (byte i = 1; i < 7; i++) {
		if (volume[i] > volume[maxBand]) {
			maxBand = i;
		}
	}
	return(maxBand);
}

uint32_t avgVol() {
	unsigned int volumeTotal = (volume[0][0] + volume[1][0] + volume[2][0] + volume[3][0] + volume[4][0] + volume[5][0] + volume[6][0]);
	return(volumeTotal / 7);
}

// Wheel written by Lady Ada for NeoPixels, adjusted for Dotstars by Jack Christensen
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel3(uint8_t WheelPos)
{
	WheelPos = 255 - WheelPos;
	if (WheelPos < 85) {
		return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
	}
	else if (WheelPos < 170) {
		WheelPos -= 85;
		return strip.Color(WheelPos * 3, 0, 255 - WheelPos * 3);
	}
	else {
		WheelPos -= 170;
		return strip.Color(255 - WheelPos * 3, WheelPos * 3 , 0);
	}
}