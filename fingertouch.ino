#include <Adafruit_MPR121.h>
#include <Adafruit_DotStar.h>
// Because conditional #includes don't work w/Arduino sketches...
#include <SPI.h>         // COMMENT OUT THIS LINE FOR GEMMA OR TRINKET
//#include <avr/power.h> // ENABLE THIS LINE FOR GEMMA OR TRINKET

Adafruit_MPR121 cap = Adafruit_MPR121();

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

#define NUMPIXELS 36 // Number of LEDs in strip

// Here's how to control the LEDs from any two pins:
#define DATAPIN    9
#define CLOCKPIN   10
Adafruit_DotStar strip = Adafruit_DotStar(
                           NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);
// The last parameter is optional -- this is the color data order of the
// DotStar strip, which has changed over time in different production runs.
// Your code just uses R,G,B colors, the library then reassigns as needed.
// Default is DOTSTAR_BRG, so change this if you have an earlier strip.

// Hardware SPI is a little faster, but must be wired to specific pins
// (Arduino Uno = pin 11 for data, 13 for clock, other boards are different).
//Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DOTSTAR_BRG);

struct Finger {
  int numPix;
  int start;
  int bufferIndex;
};

Finger fingers[] = {
  (Finger) { 3, 0, -1 },
  (Finger) { 4, 3, -1 },
  (Finger) { 4, 7, -1 },
  (Finger) { 4, 11, -1 },
  (Finger) { 3, 15, -1 },
  (Finger) { 3, 18, -1 },
  (Finger) { 4, 21, -1 },
  (Finger) { 4, 25, -1 },
  (Finger) { 4, 29, -1 },
  (Finger) { 3, 33, -1 }
};

void setup() {

#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
  clock_prescale_set(clock_div_1); // Enable 16 MHz on Trinket
#endif

  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off ASAP

  // MPR121 setup:
  Serial.begin(9600);
  Serial.println("Adafruit MPR121 Capacitive Touch sensor test");

  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");
}

uint32_t colors[] = {
  0,
  0,
  0,
  0,
  0xFFFFFF,      // 'On' color (starts white)
  0xFFFF66,
  0xFFFF00,
  0x004400,
  0,
  0,
  0,
  0
};

void loop() {
  // Get the currently touched pads
  currtouched = cap.touched();

  for (uint8_t i = 0; i < 10; i++) {
    // it if *is* touched and *wasnt* touched before, alert!
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
      fingers[i].bufferIndex = 0;
      Serial.print(i); Serial.println(" touched");
    }
    // if it *was* touched and now *isnt*, alert!
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
      Serial.print(i); Serial.println(" released");
    }
  }

  // reset our state
  lasttouched = currtouched;

  uint16_t i;
  // For each finger:
  for (i = 0; i < 10; ++i) {
    if (fingers[i].bufferIndex == -1) {
      continue;
    }
    uint16_t j;
    // For each pixel in the finger:
    for (j = 0; j < fingers[i].numPix; ++j) {
      strip.setPixelColor(j + fingers[i].start, colors[fingers[i].bufferIndex + j]);
    }
    if (fingers[i].bufferIndex == 8) {
      fingers[i].bufferIndex = -1;
    } else {
      fingers[i].bufferIndex++;
    }
  }

  strip.show();                     // Refresh strip
  delay(60);                        // Pause 20 milliseconds (~50 FPS)
}
