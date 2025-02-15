#include <Keyboard.h>
#include <ArduinoJson.h>
#include "keyset.h"

const int keyPins[5] = {6, 5, 4, 3, 2};
const int modeLeds[3] = {15, 14, 16};
const int shiftLed = 10;

Keyset keyset(
  keyPins, modeLeds, shiftLed,
  [](char pressedChar) {
    Keyboard.write(pressedChar);
  },
  [](const char* event) {
    Serial.print(event);
  }
);

void setup() {
  keyset.keysetSetup();

  Serial.begin(9600);
  Keyboard.begin();
}

void loop() {
  keyset.keysetLoop();
}
