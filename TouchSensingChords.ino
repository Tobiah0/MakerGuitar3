#include <Arduino.h>

// 8 fret pads using ESP32 built-in capacitive touch.
// These are the GPIO numbers behind the T0, T3-T9 pads on the makerboard.
const int FRET_PINS[8] = {
  4,   // Fret 1 (T0)
  15,  // Fret 2 (T3)
  13,  // Fret 3 (T4)
  12,  // Fret 4 (T5)
  14,  // Fret 5 (T6)
  27,  // Fret 6 (T7)
  33,  // Fret 7 (T8)
  32   // Fret 8 (T9)
};

// 6 string buttons on non-touch pins
const int STRING_BUTTONS[6] = {
  21, // String 0 (low E)  - SDA
  22, // String 1 (A)      - SCL
  18, // String 2 (D)      - SCK
  19, // String 3 (G)      - MISO
  23, // String 4 (B)      - MOSI
  26  // String 5 (high E) - A19
};

// Standard guitar tuning (low to high)
const int STRING_NOTES[6] = {
  40, // String 0 (low E)  = E2
  45, // String 1 (A)      = A2
  50, // String 2 (D)      = D3
  55, // String 3 (G)      = G3
  59, // String 4 (B)      = B3
  64  // String 5 (high E) = E4
};

// Auto-calibrated baseline value for each fret (measured at startup).
// A fret is considered touched when its reading drops noticeably below baseline.
int fretBaseline[8];

// How much below baseline counts as a touch (0.6 = touched if reading < 60% of baseline)
#define TOUCH_RATIO 0.6

// Track each button's pressed state and which note each string is currently playing
bool buttonPressed[6] = {false, false, false, false, false, false};
int playingNote[6] = {-1, -1, -1, -1, -1, -1};

// Track fret state so we can print only on changes
bool fretTouched[8] = {false, false, false, false, false, false, false, false};

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }

  Serial.println("\n--- Starting MakerGuitar ---");

  // Set up button pins as inputs with internal pull-up resistors.
  for (int i = 0; i < 6; i++) {
    pinMode(STRING_BUTTONS[i], INPUT_PULLUP);
  }

  // Calibrate each fret's baseline by averaging 20 readings.
  // Make sure no fingers are on the pads during this!
  Serial.println("Calibrating frets... DON'T touch the pads!");
  delay(1000);
  for (int i = 0; i < 8; i++) {
    long sum = 0;
    for (int j = 0; j < 20; j++) {
      sum += touchRead(FRET_PINS[i]);
      delay(10);
    }
    fretBaseline[i] = sum / 20;
    Serial.print("Fret ");
    Serial.print(i);
    Serial.print(" baseline = ");
    Serial.println(fretBaseline[i]);
  }
  Serial.println("Ready!");
}

// Returns true if fret i is currently being touched
bool isFretTouched(int i) {
  return touchRead(FRET_PINS[i]) < fretBaseline[i] * TOUCH_RATIO;
}

// Find the highest fret pad currently touched (closest to guitar body)
// Returns the fret number (0-7), or -1 if no fret is held
int getCurrentFret() {
  for (int i = 7; i >= 0; i--) {
    if (isFretTouched(i)) {
      return i;
    }
  }
  return -1;
}

void loop() {
  // Track each fret individually and print when state changes
  for (int i = 0; i < 8; i++) {
    bool nowTouched = isFretTouched(i);
    if (nowTouched && !fretTouched[i]) {
      Serial.print("FRET ");
      Serial.print(i);
      Serial.println(" TOUCHED");
      fretTouched[i] = true;
    } else if (!nowTouched && fretTouched[i]) {
      Serial.print("FRET ");
      Serial.print(i);
      Serial.println(" RELEASED");
      fretTouched[i] = false;
    }
  }

  // Check each button (string) for press/release events
  for (int s = 0; s < 6; s++) {
    // INPUT_PULLUP means LOW = pressed, HIGH = released
    bool currentlyPressed = (digitalRead(STRING_BUTTONS[s]) == LOW);

    if (currentlyPressed && !buttonPressed[s]) {
      // Button just pressed -- figure out what note this string should play.
      // No fret held = open string (just the base note).
      // Fret held    = base note + (fret position + 1) semitones.
      int fret = getCurrentFret();
      int note = STRING_NOTES[s] + (fret >= 0 ? fret + 1 : 0);

      Serial.print("BUTTON ");
      Serial.print(s);
      Serial.print(" PRESSED (fret=");
      Serial.print(fret);
      Serial.print(", note=");
      Serial.print(note);
      Serial.println(")");

      Serial.print("ON,");
      Serial.println(note);

      playingNote[s] = note;
      buttonPressed[s] = true;
    }
    else if (!currentlyPressed && buttonPressed[s]) {
      if (playingNote[s] != -1) {
        Serial.print("BUTTON ");
        Serial.print(s);
        Serial.println(" RELEASED");

        Serial.print("OFF,");
        Serial.println(playingNote[s]);
        playingNote[s] = -1;
      }
      buttonPressed[s] = false;
    }
  }

  delay(10); // Small delay for stability and rough button debouncing
}