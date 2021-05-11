#include <Keyboard.h>
#include "keyset.h"

void setup() {
  keysetSetup();

  Serial.begin(9600);
  Keyboard.begin();
}

void loop() {
  keysetLoop([](char pressedChar) {
    Keyboard.write(pressedChar);
  });
}
