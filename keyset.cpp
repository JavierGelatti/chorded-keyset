#include <limits.h>
#include <Arduino.h>
#include "keyset.h"

#define numKeys 5
#define maxNumberOfModes 8

class Keymap;

class Keyset {
  public:
  //const int keyPins[numKeys] = {2, 3, 4, 5, 6};
  const int keyPins[numKeys] = {3,2,4,6,5};
  //const int keyPins[numKeys] = {5,6,4,2,3};

  int pressedKeys = 0;
  long lastPressedKeyTimestamp = LONG_MAX;
  long lastReleasedKeyTimestamp = 0;
  bool shiftActivated = false;
  bool numberMode = false;

  int mode = 0;
  const int modeLeds[3] = {7, 8, 9};
  const int shiftLed = 10;

  int numberOfModes = 0;
  Keymap* keymaps[maxNumberOfModes];
};

Keyset keyset;

#define forKeyPins(key) for (int key##Index = 0, key; key##Index < numKeys && (key = keyset.keyPins[key##Index], 1); key##Index ++)
#define forindex(i, arraySize) for (int i = 0; i < arraySize; i++)


void setBit(int &n, int k) {
  n |= 1 << k;
}

void clearBit(int &n, int k) {
  n &= ~(1 << k);
}

bool testBit(int n, int k) {
  return (n & (1 << k)) != 0;
}

class Keymap {
  private:
  class SingleKeyKeymap; // Defined later
  SingleKeyKeymap* keymap[32];

  public:
  Keymap(char (&keymap)[32], bool (&modifierFlags)[32]) {
    forindex(i, 32) {
      this->keymap[i] = new SingleKeyKeymap(keymap[i], modifierFlags[i]);
    }
  }

  char pressedCharFor(int pressedKeys) {
    return keymap[pressedKeys]->character;
  }

  private:
  class SingleKeyKeymap {
    public:
    const char character;
    const bool isModifier;

    public:
    SingleKeyKeymap(char character, bool isModifier) : character(character), isModifier(isModifier) {}
  };
};


int getNumberOfModes() {
    return keyset.numberOfModes;
}

const int shift = 0b00111;
const int switchMode = 0b11100;

void registerKeymap(char (&keymap)[32], bool (&modifiers)[32]) {
  keyset.keymaps[keyset.numberOfModes++] = new Keymap(keymap, modifiers);
}

void registerKeymap(char (&keymap)[32]) {
  bool noModifiers[32] = { false };
  registerKeymap(keymap, noModifiers);
}

void initializeKeymap() {
  // Normal keymap
  char keymap[32] = {0};

  // One finger down
  keymap[0b00001] = 'a';
  keymap[0b00010] = 'e';
  keymap[0b00100] = 'i';
  keymap[0b01000] = 'o';
  keymap[0b10000] = 'u';

  // Two fingers together
  keymap[0b00011] = 'n';
  keymap[0b00110] = 't';
  keymap[0b01100] = 'l';
  keymap[0b11000] = 's';

  // Two fingers skipping one
  keymap[0b00101] = 'd';
  keymap[0b01010] = 'm';
  keymap[0b10100] = 'z';

  // Two fingers skipping two
  keymap[0b01001] = 'k';
  keymap[0b10010] = 'b';

  // One finger up
  keymap[0b11110] = 'h';
  keymap[0b11101] = 'c';
  keymap[0b11011] = 'v';
  keymap[0b10111] = 'j';
  keymap[0b01111] = 'r';

  // Looks like y and w
  keymap[0b10001] = 'y';
  keymap[0b10101] = 'w';

  // One finger chasing two
  keymap[0b01101] = 'g';
  keymap[0b11010] = 'x';

  // Two fingers chasing one
  keymap[0b01011] = 'f';
  keymap[0b10110] = 'q';

  // Looks like ping pong | three fingers skipping two
  keymap[0b10011] = 'p';
  keymap[0b11001] = KEY_RETURN;

  // Three fingers together
  keymap[0b00111] = '\0'; // Shift
  keymap[0b01110] = KEY_BACKSPACE;
  keymap[0b11100] = '\0'; // Switch

  // All fingers
  keymap[0b11111] = ' ';

  registerKeymap(keymap);

  // Number keymap
  char numberKeymap[32] = {0};

  // 1 through 5
  numberKeymap[0b00001] = '1';
  numberKeymap[0b00010] = '2';
  numberKeymap[0b00100] = '3';
  numberKeymap[0b01000] = '4';
  numberKeymap[0b10000] = '5';

  // Rest of numbers
  numberKeymap[0b10001] = '6';
  numberKeymap[0b10010] = '7';
  numberKeymap[0b10100] = '8';
  numberKeymap[0b11000] = '9';
  numberKeymap[0b11110] = '0'; // h

  numberKeymap[0b11101] = KEY_ESC; // c

  registerKeymap(numberKeymap);
}

void keysetSetup() {
  initializeKeymap();

  forKeyPins(keyPin) {
    pinMode(keyPin, INPUT_PULLUP);
  }
  for (int i = 0; i < 3; i++) {
    pinMode(keyset.modeLeds[i], OUTPUT);
  }
  pinMode(keyset.shiftLed, OUTPUT);
}

void clearPressedKeys() {
  keyset.pressedKeys = 0;
  keyset.lastPressedKeyTimestamp = LONG_MAX;
}

void clearPressedKey(int keyIndex) {
  clearBit(keyset.pressedKeys, keyIndex);
}

bool isBeingPressed(int keyPin) {
  int yes = 0;
  const int tries = 1000;
  for (int i = 0; i < tries; i++) {
    yes += (digitalRead(keyPin) == 0) ? 1 : -1;
  }
  //if (yes != tries && yes != -tries) {
  //  Serial.print("HEHE: ");
  //  Serial.println(yes);
  //}
  return yes > 0;
}

bool wasPressed(int keyIndex) {
  return testBit(keyset.pressedKeys, keyIndex);
}

long ellapsedTimeFrom(long milliseconds) {
  return millis() - milliseconds;
}

void loopWithDelayForPress(std::function<void(char)>);

void keysetLoop(std::function<void(char)> keyboardWrite) {
  loopWithDelayForPress(keyboardWrite);

  for (int i = 0; i < 3; i++) {
    digitalWrite(keyset.modeLeds[i], testBit(keyset.mode+1, i));
  }
  digitalWrite(keyset.shiftLed, keyset.shiftActivated);
}

void loopWithDelayForPress(std::function<void(char)> keyboardWrite) {
  bool nothingPressed = true;
  forKeyPins(keyPin) {
    if (isBeingPressed(keyPin)) {
      setBit(keyset.pressedKeys, numKeys - 1 - keyPinIndex); // Esto cambia entre izq y der
      keyset.lastPressedKeyTimestamp = millis();
      nothingPressed = false;
    } else {
    }
  }

  if (nothingPressed && keyset.pressedKeys != 0) {
    char pressedChar;
    pressedChar = keyset.keymaps[keyset.mode]->pressedCharFor(keyset.pressedKeys);
    if (keyset.shiftActivated) pressedChar = toupper(pressedChar);

    keyboardWrite(pressedChar);

    if (keyset.pressedKeys == shift) {
      keyset.shiftActivated = !keyset.shiftActivated;
    } else if (keyset.pressedKeys == switchMode) {
      keyset.mode = (keyset.mode + 1) % keyset.numberOfModes;
    }
    clearPressedKeys();
  }
}