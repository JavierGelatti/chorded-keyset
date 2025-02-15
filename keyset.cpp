#include <limits.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "keyset.h"
#include "bit_manipulation.h"
#include "json_utils.h"

#define forindex(i, arraySize) for (int i = 0; i < arraySize; i++)

String readLine();

class Keymap {
  private:
  class SingleKeyKeymap; // Defined later
  SingleKeyKeymap* keymap[32];

  public:
  Keymap(char (&keymap)[32]) {
    bool noModifiers[32] = { false };
    initialize(keymap, noModifiers);
  }
  Keymap(char (&keymap)[32], bool (&modifierFlags)[32]) {
    initialize(keymap, modifierFlags);
  }
  ~Keymap() {
    forindex(i, 32) {
      delete keymap[i];
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

  // Special characters
  numberKeymap[0b11111] = ' ';
  numberKeymap[0b01110] = KEY_BACKSPACE;
  numberKeymap[0b11001] = KEY_RETURN;
  numberKeymap[0b11101] = KEY_ESC; // c

  numberKeymap[0b00111] = '\0'; // Shift
  numberKeymap[0b11100] = '\0'; // Switch

  return new Keymap(numberKeymap);
}

const int shift = 0b00111;
const int switchMode = 0b11100;

#define forKeyPins(key) for (int key##Index = 0, key; key##Index < numKeys && (key = this->keyPins[key##Index], 1); key##Index ++)

int Keyset::getNumberOfModes() {
    return numberOfKeymaps;
}

void Keyset::keysetSetup() {
  initializeKeymap();

  forKeyPins(keyPin) {
    pinMode(keyPin, INPUT_PULLUP);
  }
  for (int i = 0; i < numModeLeds; i++) {
    pinMode(modeLeds[i], OUTPUT);
  }
  pinMode(shiftLed, OUTPUT);
}

void Keyset::keysetLoop() {
  loopWithDelayForPress();
  processIncomingCommand();

  for (int i = 0; i < numModeLeds; i++) {
    digitalWrite(modeLeds[i], currentMode == i ? HIGH : LOW);
  }
  digitalWrite(shiftLed, shiftActivated);
}

void Keyset::registerKeymap(Keymap* keymapToRegister) {
  keymaps[numberOfKeymaps++] = keymapToRegister;
}

void Keyset::initializeKeymap() {
  registerKeymap(normalKeymap());
  registerKeymap(numberKeymap());
}

void Keyset::clearPressedKeys() {
  pressedKeys = 0;
  lastPressedKeyTimestamp = LONG_MAX;
}

void Keyset::clearPressedKey(int keyIndex) {
  clearBit(pressedKeys, keyIndex);
}

bool Keyset::isBeingPressed(int keyPin) {
  int yes = 0;
  const int tries = 1000;
  for (int i = 0; i < tries; i++) {
    yes += (digitalRead(keyPin) == 0) ? 1 : -1;
  }
  return yes > 0;
}

bool Keyset::wasPressed(int keyIndex) {
  return testBit(pressedKeys, keyIndex);
}

long Keyset::ellapsedTimeFrom(long milliseconds) {
  return millis() - milliseconds;
}

char Keyset::chordedCharacter() {
  int keymapIndex = isInLogMode() ? 0 : currentMode;
  char pressedChar = keymaps[keymapIndex]->pressedCharFor(pressedKeys);
  return shiftActivated ? toupper(pressedChar) : pressedChar;
}

bool Keyset::isInLogMode() {
  return currentMode == numberOfKeymaps;
}

void Keyset::processChord() {
  char chordedChar = chordedCharacter();
  if (isInLogMode()) {
    if (chordedChar != '\0') logEvent(String(chordedChar).c_str());
  } else {
    if (chordedChar != '\0') keyboardWrite(chordedChar);
  }

  if (pressedKeys == shift) {
    shiftActivated = !shiftActivated;
  } else if (pressedKeys == switchMode) {
    currentMode += 1;
    currentMode %= numberOfKeymaps
      + 1 /* consider log currentMode */;
  } else {
    shiftActivated = false;
  }
  clearPressedKeys();
}

void Keyset::loopWithDelayForPress() {
  bool nothingPressed = true;
  forKeyPins(keyPin) {
    if (isBeingPressed(keyPin)) {
      setBit(
        pressedKeys,
        keyPinIndex // for right hand: numKeys - 1 - keyPinIndex
      );
      lastPressedKeyTimestamp = millis();
      nothingPressed = false;
    }
  }

  if (nothingPressed && pressedKeys != 0) {
    processChord();
  }
}

void Keyset::processIncomingCommand() {
  String commandString = readLine();
  if (commandString == "") return;

  StaticJsonDocument<200> commandDocument;
  DeserializationError error = deserializeJson(commandDocument, commandString);
  if (error) {
    // TODO: Report the error as JSON
    Serial.print("JSON parsing error: ");
    Serial.println(error.f_str());

    return;
  }

  String commandName = String((const char*) commandDocument["command"]);
  if (commandName == "log_mode") {
    currentMode = 2;
  } else if (commandName == "normal_mode") {
    currentMode = 0;
  } else if (commandName == "query_state") {
    StaticJsonDocument<200> responseDocument;
    responseDocument["mode"] = currentMode;
    Serial.println(serializeJson(responseDocument));
  }
}

String readLine() {
  String receivedLine = "";

  while (Serial.available()) {
    char readChar = Serial.read();
    if (readChar == '\n') break;

    receivedLine += readChar;
  }

  return receivedLine;
}

Keyset::Keyset(
  const int (&keyPins)[numKeys],
  const int (&modeLeds)[numModeLeds],
  const int shiftLed,
  const std::function<void(char)> keyboardWrite,
  const std::function<void(const char*)> logEvent
) : // We initialize the arrays like this to avoid an "array as initializer" error
  keyPins{keyPins[0], keyPins[1], keyPins[2], keyPins[3], keyPins[4]},
  modeLeds{modeLeds[0], modeLeds[1], modeLeds[2]},
  shiftLed(shiftLed),
  keyboardWrite(keyboardWrite),
  logEvent(logEvent)
{
  this->pressedKeys = 0;
  this->lastPressedKeyTimestamp = LONG_MAX;
  this->lastReleasedKeyTimestamp = 0;
  this->shiftActivated = false;
  this->numberMode = false;
  this->currentMode = 0;
  this->numberOfKeymaps = 0;
}

Keyset::~Keyset() {
  forindex(i, numberOfKeymaps) {
    delete keymaps[i];
  }
}
