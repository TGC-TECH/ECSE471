#include <Arduino.h>

#define BUTTON_PIN     2
#define TRIAC_PIN      3
#define ZERO_CROSS_PIN 4

// 7-Segment GPIO pins (A–G)
const uint8_t segmentPins[7] = {5, 6, 7, 8, 9, 10, 11};

// Segment encoding for 0–9 (common annode: PIN HIGH = OFF)
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

#define LONG_PRESS_DURATION 1000     // ms
#define DISPLAY_TIME        1000     // ms
#define HEATING_TIMEOUT     30000    // ms

enum State { IDLE, SET_TEMP, HEATING };
State state = IDLE;

volatile bool zeroCross = false;

int temp = 1;
unsigned long lastPressTime = 0;
unsigned long displayStartTime = 0;
unsigned long heatingStartTime = 0;

bool buttonPressed = false;

// Duty cycles for temp settings 1–9 (%)
uint8_t dutyCycle[10] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90};

void showTemp(int t) {
  if (t < 0 || t > 9) return;
  for (int i = 0; i < 7; i++) {
        // Common anode: LOW = ON, HIGH = OFF
    digitalWrite(segmentPins[i], segmentMap[t][i] ? LOW : HIGH);
  }
}

void clearDisplay() {
  for (int i = 0; i < 7; i++) {
    digitalWrite(segmentPins[i], HIGH);
  }
}


void fireTriac() {
  digitalWrite(TRIAC_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIAC_PIN, LOW);
}

void onZeroCross() {
  zeroCross = true;
}

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(TRIAC_PIN, OUTPUT);
  pinMode(ZERO_CROSS_PIN, INPUT);

  for (int i = 0; i < 7; i++) {
    pinMode(segmentPins[i], OUTPUT);
    digitalWrite(segmentPins[i], LOW);
  }

  attachInterrupt(digitalPinToInterrupt(ZERO_CROSS_PIN), onZeroCross, RISING);
  digitalWrite(TRIAC_PIN, LOW);

  showTemp(0);
}



void loop() {
  static bool lastButtonState = HIGH;
  bool currentButtonState = digitalRead(BUTTON_PIN);

  // Button press detection
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    lastPressTime = millis();
    buttonPressed = true;
  }

  // Button release detection
  if (lastButtonState == LOW && currentButtonState == HIGH) {
    unsigned long pressDuration = millis() - lastPressTime;
    buttonPressed = false;

    if (pressDuration < LONG_PRESS_DURATION) {
      // Short press: Change temp
      if (state != HEATING) {
        temp++;
        if (temp > 9) temp = 1;
        Serial.println(temp);
        displayStartTime = millis();
        state = SET_TEMP;
        showTemp(temp);
      }
    } else {
      // Long press: toggle heating
      if (state == HEATING) {
        Serial.println("Heating cancelled");
        state = IDLE;
        showTemp(0);
      } else {
        Serial.println("Heating started");
        state = HEATING;
        heatingStartTime = millis();
        showTemp(temp);
      }
    }
  }

  lastButtonState = currentButtonState;

  // Handle zero-cross interrupt for TRIAC
  if (zeroCross && state == HEATING) {
    zeroCross = false;
    int delayMicro = map(100 - dutyCycle[temp], 0, 100, 1000, 9000);  // Adjust for your power
    delayMicroseconds(delayMicro);
    fireTriac();
  }

  // Auto-clear temp display
  if (state == SET_TEMP && millis() - displayStartTime > DISPLAY_TIME) {
    showTemp(0);
    state = IDLE;
  }

  // Heating timeout
  if (state == HEATING && millis() - heatingStartTime > HEATING_TIMEOUT) {
    Serial.println("Heating timed out");
    state = IDLE;
    showTemp(0);
  }
}
