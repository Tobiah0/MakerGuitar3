#include <Wire.h>
#include "Adafruit_MPR121.h"

Adafruit_MPR121 neckSensor = Adafruit_MPR121();

// Track the overall state of the sensor
uint16_t lastTouched = 0;

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
  
  // Default thresholds
  neckSensor.setThresholds(6, 4);
}

void loop() {
  // touched() returns a 16-bit number. 
  // If it's exactly 0, nothing is touched. If it's greater than 0, something is touched.
  uint16_t currentTouched = neckSensor.touched();

  // Only print something if the state actually changed (to avoid spamming the console)
  if (currentTouched != lastTouched) {
    
    if (currentTouched > 0) {
      Serial.println("👉 TOUCH DETECTED!");
    } else {
      Serial.println("🖐️ All pins released.");
    }
    
    lastTouched = currentTouched;
  }
  
  delay(50); // Small delay for stability
}