#include <Wire.h>
#include "Adafruit_MPR121.h"

Adafruit_MPR121 neckSensor = Adafruit_MPR121();

// Track the overall state of the sensor
uint16_t lastTouched = 0;

// If a pin's filtered value drops below this, we consider it touched
// We know untouched is ~22 and touched is ~3, so 10 is a safe midpoint
#define TOUCH_THRESHOLD 10

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); } 
  
  Serial.println("\n--- Starting Basic Touch Test ---");

  // Initialize the MPR121
  if (!neckSensor.begin(0x5A)) {
    Serial.println("ERROR: MPR121 not found! Check your I2C wiring.");
    while (1); // Halt the board here if sensor fails
  }
  
  Serial.println("SUCCESS: MPR121 found! Go ahead and touch a pin.");
  
  neckSensor.setThresholds(2, 1);
}

void loop() {
  // Print raw electrode values so we can see what's happening when you touch
  //Serial.print("Electrodes: ");
  //for (int i = 0; i < 12; i++) {
   // Serial.print(neckSensor.filteredData(i));
   // Serial.print("\t");
  //}
  //Serial.println();

  // Build our own touched bitmask by checking filteredData directly,
  // since touched() seems to be missing signals despite clear value drops
  uint16_t currentTouched = 0;
  for (int i = 0; i < 12; i++) {
    if (neckSensor.filteredData(i) < TOUCH_THRESHOLD) {
      currentTouched |= (1 << i);
    }
  }

  // Only print something if the state actually changed (to avoid spamming the console)
  if (currentTouched != lastTouched) {
    
    if (currentTouched > 0) {
      Serial.println("TOUCH DETECTED!");
    } else {
      Serial.println("All pins released.");
    }
    
    lastTouched = currentTouched;
  }
  
  delay(50); // Small delay for stability
}
