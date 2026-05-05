#include <Wire.h>
#include "Adafruit_MPR121.h"

Adafruit_MPR121 neckSensor = Adafruit_MPR121();

// Track the overall state of the sensor
uint16_t lastTouched = 0;

// If a pin's filtered value drops below this, we consider it touched
// We know untouched is ~22 and touched is ~3, so 10 is a safe midpoint
#define TOUCH_THRESHOLD 10

// MIDI note mapping for each fret (pins 0-11)
// CAN ALWAYS CHANGE (TBD) , lets say we want to play a song or sum.
const int MIDI_NOTES[12] = {
  40, // Pin 0  = E2
  43, // Pin 1  = G2
  45, // Pin 2  = A2
  47, // Pin 3  = B2
  50, // Pin 4  = D3
  52, // Pin 5  = E3
  55, // Pin 6  = G3
  57, // Pin 7  = A3
  59, // Pin 8  = B3
  62, // Pin 9  = D4
  64, // Pin 10 = E4
  67  // Pin 11 = G4
};

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }

  Serial.println("\n--- Starting MakerGuitar ---");

  // Initialize the MPR121
  if (!neckSensor.begin(0x5A)) {
    Serial.println("ERROR: MPR121 not found! Check your I2C wiring.");
    while (1); // Halt the board here if sensor fails
  }

  Serial.println("SUCCESS: MPR121 found! Go ahead and touch a fret.");

  neckSensor.setThresholds(2, 1);
}

void loop() {
  // Ignore the first 2 seconds after boot so the MPR121 baseline can settle.
  // Without this, pins briefly cross the threshold during calibration and fire
  // phantom touches as soon as the program starts.
  if (millis() < 2000) {
    return;
  }

  // checking filteredData directly,
  // since touched() seems to be missing signals despite clear value drops
  uint16_t currentTouched = 0;
  for (int i = 0; i < 12; i++) {
    if (neckSensor.filteredData(i) < TOUCH_THRESHOLD) {
      currentTouched |= (1 << i);
    }
  }

  // Only send serial messages if the state actually changed
  if (currentTouched != lastTouched) {
    for (int i = 0; i < 12; i++) {
      bool wasTouched = (lastTouched >> i) & 1;
      bool isTouched  = (currentTouched >> i) & 1;

      if (!wasTouched && isTouched) {
        // Fret just pressed 
        Serial.print("ON,");
        Serial.println(MIDI_NOTES[i]);
      } else if (wasTouched && !isTouched) {
        // Fret just released
        Serial.print("OFF,");
        Serial.println(MIDI_NOTES[i]);
      }
    }
    lastTouched = currentTouched;
  }

  delay(50); // Small delay for stability
}