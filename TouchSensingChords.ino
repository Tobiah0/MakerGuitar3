#include <Wire.h>
#include "Adafruit_MPR121.h"

Adafruit_MPR121 neckSensor = Adafruit_MPR121();

// String Button Configuration
const int NUM_STRINGS = 6;
// Connect buttons to these pins and ground. We will use internal pull-up resistors.
const int stringPins[NUM_STRINGS] = {12, 13, 14, 25, 26, 27}; 

// MIDI note for each open string, matched 1:1 with stringPins
// Standard tuning low to high: E2, A2, D3, G3, B3, E4
const int openStringNotes[NUM_STRINGS] = {40, 45, 50, 55, 59, 64};

// Track the state of each string button
int lastStringState[NUM_STRINGS] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};

// Track currently playing notes so we can send correct "Note Off" commands
int currentlyPlayingNotes[NUM_STRINGS] = {-1, -1, -1, -1, -1, -1};

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); } // Wait for serial console
  
  Serial.println("Maker Guitar Initializing...");

  // Initialize the MPR121
  if (!neckSensor.begin(0x5A)) {
    Serial.println("MPR121 not found");
    while (1);
  }
  
  // Set touch and release thresholds. 
  // Default is 12/6. Copper tape might need higher values (e.g., 20/10) if you get false triggers.
  neckSensor.setThresholds(12, 6);

  // Initialize string button pins
  for (int i = 0; i < NUM_STRINGS; i++) {
    pinMode(stringPins[i], INPUT_PULLUP);
  }
}

void loop() {
  // get current pressed frets
  uint16_t touchedFrets = neckSensor.touched();
  int activeFret = getHighestFret(touchedFrets);

  // Loop through string buttons and playNotes(string_button_press, activeFret)
  for (int s = 0; s < NUM_STRINGS; s++) {
    int currentState = digitalRead(stringPins[s]);
    if (currentState == LOW && lastStringState[s] == HIGH) {
      int note = openStringNotes[s] + activeFret;

      // If a previous note was still ringing on this string, kill it first
      if (currentlyPlayingNotes[s] != -1) {
        serialNoteOff(currentlyPlayingNotes[s]);
      }

      serialNoteOn(note);
      currentlyPlayingNotes[s] = note;

    
    // TODO: Note playing logic
    // Concept: Send serial data that note is on so computer can play it
      
      delay(5); // Tiny debounce delay
    } 
    // Check for button release
    else if (currentState == HIGH && lastStringState[s] == LOW) {
      if (currentlyPlayingNotes[s] != -1) {
        serialNoteOff(currentlyPlayingNotes[s]);
        currentlyPlayingNotes[s] = -1;
      }

      delay(5); // Tiny debounce delay
    }

    lastStringState[s] = currentState;
  }
}


//Returns highest fret currently being touched
int getHighestFret(uint16_t touchedData) {
    for (int i = 11; i >= 0; i--) {
      if (touchedData & (1 << i)) {
        return i + 1;
      }
    }
    return 0;
}
void serialNoteOn(int note) {
  Serial.print("ON,");
  Serial.println(note);
}

void serialNoteOff(int note) {
  Serial.print("OFF,");
  Serial.println(note);
}