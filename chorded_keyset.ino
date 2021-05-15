#include <Keyboard.h>
#include "keyset.h"

const int keyPins[5] = {5,6,4,2,3}; //{2, 3, 4, 5, 6};
const int modeLeds[] = {7, 8, 9};

Keyset keyset(keyPins, modeLeds);

void setup() {
  keyset.keysetSetup();

  Serial.begin(9600);
  Keyboard.begin();
}

void loop() {
  keyset.keysetLoop([](char pressedChar) {
    Keyboard.write(pressedChar);
  });
}
