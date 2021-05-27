#include <Keyboard.h>
#include "keyset.h"

const int keyPins[5] = {5, 6, 4, 2, 3};
const int modeLeds[3] = {15, 14, 16};
const int shiftLed = 10;
const int switchPins[2] = {9, 8};

Keyset keyset(
  keyPins, modeLeds, shiftLed, switchPins,
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
