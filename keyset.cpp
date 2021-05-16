#include <limits.h>
#include <Arduino.h>
#include "keyset.h"
#include "bit_manipulation.h"

#define forindex(i, arraySize) for (int i = 0; i < arraySize; i++)

class Keymap {
  private:
  class SingleKeyKeymap; // Defined later
  SingleKeyKeymap* keymap[32];

  public:
  Keymap(char (&keymap)[32]) {
    bool noModifiers[32] = { false };
    this->initialize(keymap, noModifiers);
  }
  Keymap(char (&keymap)[32], bool (&modifierFlags)[32]) {
    this->initialize(keymap, modifierFlags);
  }
  ~Keymap() {
    forindex(i, 32) {
      delete this->keymap[i];
    }
  }

  char pressedCharFor(int pressedKeys) {
    return keymap[pressedKeys]->character;
  }

  private:

  void initialize(char (&keymap)[32], bool (&modifierFlags)[32]) {
    forindex(i, 32) {
      this->keymap[i] = new SingleKeyKeymap(keymap[i], modifierFlags[i]);
    }
  }

  class SingleKeyKeymap {
    public:
    const char character;
    const bool isModifier;

    public:
    SingleKeyKeymap(char character, bool isModifier) : character(character), isModifier(isModifier) {}
  };
};

Keymap* normalKeymap() {
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

  return new Keymap(keymap);
}

Keymap* numberKeymap() {
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

  return new Keymap(numberKeymap);
}

const int shift = 0b00111;
const int switchMode = 0b11100;

#define forKeyPins(key) for (int key##Index = 0, key; key##Index < numKeys && (key = this->keyPins[key##Index], 1); key##Index ++)

int Keyset::getNumberOfModes() {
    return this->numberOfModes;
}

void Keyset::keysetSetup() {
  initializeKeymap();

  forKeyPins(keyPin) {
    pinMode(keyPin, INPUT_PULLUP);
  }
  for (int i = 0; i < 3; i++) {
    pinMode(this->modeLeds[i], OUTPUT);
  }
  pinMode(this->shiftLed, OUTPUT);
}

void Keyset::keysetLoop() {
  loopWithDelayForPress();

  for (int i = 0; i < 3; i++) {
    digitalWrite(this->modeLeds[i], testBit(this->mode+1, i));
  }
  digitalWrite(this->shiftLed, this->shiftActivated);
}

void Keyset::registerKeymap(Keymap* keymapToRegister) {
  this->keymaps[this->numberOfModes++] = keymapToRegister;
}

void Keyset::initializeKeymap() {
  registerKeymap(normalKeymap());
  registerKeymap(numberKeymap());
}

void Keyset::clearPressedKeys() {
  this->pressedKeys = 0;
  this->lastPressedKeyTimestamp = LONG_MAX;
}

void Keyset::clearPressedKey(int keyIndex) {
  clearBit(this->pressedKeys, keyIndex);
}

bool Keyset::isBeingPressed(int keyPin) {
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

bool Keyset::wasPressed(int keyIndex) {
  return testBit(this->pressedKeys, keyIndex);
}

long Keyset::ellapsedTimeFrom(long milliseconds) {
  return millis() - milliseconds;
}

void Keyset::loopWithDelayForPress() {
  bool nothingPressed = true;
  forKeyPins(keyPin) {
    if (isBeingPressed(keyPin)) {
      setBit(this->pressedKeys, keyPinIndex); // Esto cambia entre izq y der (numKeys-1-keyPinIndex)
      this->lastPressedKeyTimestamp = millis();
      nothingPressed = false;
    } else {
    }
  }

  if (nothingPressed && this->pressedKeys != 0) {
    char pressedChar;
    pressedChar = this->keymaps[this->mode]->pressedCharFor(this->pressedKeys);
    if (this->shiftActivated) pressedChar = toupper(pressedChar);

    if (pressedChar != '\0') keyboardWrite(pressedChar);

    if (this->pressedKeys == shift) {
      this->shiftActivated = !this->shiftActivated;
    } else if (this->pressedKeys == switchMode) {
      this->mode = (this->mode + 1) % this->numberOfModes;
    }
    clearPressedKeys();
  }
}

Keyset::Keyset(const int (&keyPins)[numKeys], const int (&modeLeds)[3], const std::function<void(char)> keyboardWrite)
  // We initialize the arrays like this to avoid an "array as initializer" error
  : keyPins{keyPins[0], keyPins[1], keyPins[2], keyPins[3], keyPins[4]},
    modeLeds{modeLeds[0], modeLeds[1], modeLeds[2]},
    shiftLed(10),
    keyboardWrite(keyboardWrite) {
  this->pressedKeys = 0;
  this->lastPressedKeyTimestamp = LONG_MAX;
  this->lastReleasedKeyTimestamp = 0;
  this->shiftActivated = false;
  this->numberMode = false;
  this->mode = 0;
  this->numberOfModes = 0;
}

Keyset::~Keyset() {
  forindex(i, numberOfModes) {
    delete keymaps[i];
  }
}
