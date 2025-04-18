#include <Arduino.h>

// Segment pins connected to GPIOs (A to G)
const uint8_t segmentPins[7] = {5, 6, 7, 8, 9, 10, 11};

// Digit segment map (common anode: 1 = ON → LOW output)
const uint8_t segmentMap[10][7] = {
  {1,1,1,1,1,1,0},  // 0
  {0,1,1,0,0,0,0},  // 1
  {1,1,0,1,1,0,1},  // 2
  {1,1,1,1,0,0,1},  // 3
  {0,1,1,0,0,1,1},  // 4
  {1,0,1,1,0,1,1},  // 5
  {1,0,1,1,1,1,1},  // 6
  {1,1,1,0,0,0,0},  // 7
  {1,1,1,1,1,1,1},  // 8
  {1,1,1,1,0,1,1}   // 9
};

void showDigit(int digit) {
  if (digit < 0 || digit > 9) return;

  for (int i = 0; i < 7; i++) {
    // Common anode: LOW = ON, HIGH = OFF
    digitalWrite(segmentPins[i], segmentMap[digit][i] ? LOW : HIGH);
  }
}

void setup() {
  for (int i = 0; i < 7; i++) {
    pinMode(segmentPins[i], OUTPUT);
    digitalWrite(segmentPins[i], HIGH); // start with all segments OFF
  }
}

void loop() {
  for (int i = 0; i < 7; i++) {
    // Common anode: LOW = ON, HIGH = OFF
      for (int j = 0; j < 7; j++) {
    //digitalWrite(segmentPins[j], HIGH);
  }
    //digitalWrite(segmentPins[i], LOW);
    //delay(3000);
  }


  for (int digit = 0; digit <= 9; digit++) {
    showDigit(digit);
    delay(3000); // wait 1 second
  }
}
