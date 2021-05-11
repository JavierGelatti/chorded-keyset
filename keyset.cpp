#include <limits.h>
#include <Arduino.h>

#define KEY_LEFT_CTRL   0x80
#define KEY_LEFT_SHIFT    0x81
#define KEY_LEFT_ALT    0x82
#define KEY_LEFT_GUI    0x83
#define KEY_RIGHT_CTRL    0x84
#define KEY_RIGHT_SHIFT   0x85
#define KEY_RIGHT_ALT   0x86
#define KEY_RIGHT_GUI   0x87

#define KEY_UP_ARROW    0xDA
#define KEY_DOWN_ARROW    0xD9
#define KEY_LEFT_ARROW    0xD8
#define KEY_RIGHT_ARROW   0xD7
#define KEY_BACKSPACE   0xB2
#define KEY_TAB       0xB3
#define KEY_RETURN      0xB0
#define KEY_ESC       0xB1
#define KEY_INSERT      0xD1
#define KEY_DELETE      0xD4
#define KEY_PAGE_UP     0xD3
#define KEY_PAGE_DOWN   0xD6
#define KEY_HOME      0xD2
#define KEY_END       0xD5
#define KEY_CAPS_LOCK   0xC1
#define KEY_F1        0xC2
#define KEY_F2        0xC3
#define KEY_F3        0xC4
#define KEY_F4        0xC5
#define KEY_F5        0xC6
#define KEY_F6        0xC7
#define KEY_F7        0xC8
#define KEY_F8        0xC9
#define KEY_F9        0xCA
#define KEY_F10       0xCB
#define KEY_F11       0xCC
#define KEY_F12       0xCD
#define KEY_F13       0xF0
#define KEY_F14       0xF1
#define KEY_F15       0xF2
#define KEY_F16       0xF3
#define KEY_F17       0xF4
#define KEY_F18       0xF5
#define KEY_F19       0xF6
#define KEY_F20       0xF7
#define KEY_F21       0xF8
#define KEY_F22       0xF9
#define KEY_F23       0xFA
#define KEY_F24       0xFB

#define numKeys 5

const int keyPins[numKeys] = {2, 3, 4, 5, 6};
int pressedKeys = 0;
long lastPressedKeyTimestamp = LONG_MAX;
long lastReleasedKeyTimestamp = 0;
bool shiftActivated = false;
bool numberMode = false;

int mode = 0;

#define forKeyPins(key) for (int key##Index = 0, key; key##Index < numKeys && (key = keyPins[key##Index], 1); key##Index ++)
#define forindex(i, arraySize) for (int i = 0; i < arraySize; i++)

const int modeLeds[] = {7, 8, 9};
const int shiftLed = 10;

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

int numberOfModes = 0;
const int maxNumberOfModes = 8;
Keymap* keymaps[maxNumberOfModes];

const int shift = 0b00111;
const int switchMode = 0b11100;

void registerKeymap(char (&keymap)[32], bool (&modifiers)[32]) {
  keymaps[numberOfModes++] = new Keymap(keymap, modifiers);
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
    pinMode(modeLeds[i], OUTPUT);
  }
  pinMode(shiftLed, OUTPUT);
}

void clearPressedKeys() {
  pressedKeys = 0;
  lastPressedKeyTimestamp = LONG_MAX;
}

void clearPressedKey(int keyIndex) {
  clearBit(pressedKeys, keyIndex);
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
  return testBit(pressedKeys, keyIndex);
}

long ellapsedTimeFrom(long milliseconds) {
  return millis() - milliseconds;
}

void printArray(int array[], int size) {
  Serial.print("[");
  for (int i = 0; i < size; i++) {
    Serial.print(array[i]);
    if (i < size-1) Serial.print(", ");
  }
  Serial.println("]");
}

void printArray(char array[], int size) {
  Serial.print("[");
  for (int i = 0; i < size; i++) {
    Serial.print(array[i]);
    if (i < size-1) Serial.print(", ");
  }
  Serial.println("]");
}

void loopWithDelayForPress(void (*keyboardWrite)(char));

void keysetLoop(void (*keyboardWrite)(char)) {
  loopWithDelayForPress(keyboardWrite);

  for (int i = 0; i < 3; i++) {
    digitalWrite(modeLeds[i], testBit(mode+1, i));
  }
  digitalWrite(shiftLed, shiftActivated);
}

void loopWithDelayForPress(void (*keyboardWrite)(char)) {
  bool nothingPressed = true;
  forKeyPins(keyPin) {
    if (isBeingPressed(keyPin)) {
      setBit(pressedKeys, numKeys - 1 - keyPinIndex); // Esto cambia entre izq y der
      lastPressedKeyTimestamp = millis();
      nothingPressed = false;
    }
  }

  if (nothingPressed && pressedKeys != 0) {
    char pressedChar;
    pressedChar = keymaps[mode]->pressedCharFor(pressedKeys);
    if (shiftActivated) pressedChar = toupper(pressedChar);

    keyboardWrite(pressedChar);

    if (pressedKeys == shift) {
      shiftActivated = !shiftActivated;
    } else if (pressedKeys == switchMode) {
      mode = (mode + 1) % numberOfModes;
    }
    clearPressedKeys();
  }
}

void loopWithDelayForPress2() {
  forKeyPins(keyPin) {
    if (isBeingPressed(keyPin)) {
      setBit(pressedKeys, numKeys - 1 - keyPinIndex);
      lastPressedKeyTimestamp = millis();
    }
    //Serial.print(keyPinIndex);
    //Serial.print(":");
    //Serial.println(digitalRead(keyPin));
  }

  if (ellapsedTimeFrom(lastPressedKeyTimestamp) > 20) {
    //Serial.println(pressedKeys);

    char pressedChar;
    pressedChar = keymaps[mode]->pressedCharFor(pressedKeys);
    if (shiftActivated) pressedChar = toupper(pressedChar);

    //Serial.println(pressedChar);
    //Keyboard.write(pressedChar);

    if (pressedKeys == shift) {
      shiftActivated = !shiftActivated;
    } else if (pressedKeys == switchMode) {
      mode = (mode + 1) % numberOfModes;

      //Keyboard.press(KEY_LEFT_ALT);
      //Keyboard.write(KEY_TAB);
      //Keyboard.release(KEY_LEFT_ALT);

    }
    clearPressedKeys();
  }
  //delay(100);
}

void loopWithKeyUpTrigger() {
  forKeyPins(keyPin) {
    bool beingPressed = isBeingPressed(keyPin);
    if (beingPressed && !wasPressed(keyPinIndex)) {
      //Serial.print("Pushed: ");
      //Serial.println(keyPinIndex);
      setBit(pressedKeys, keyPinIndex);
      lastPressedKeyTimestamp = millis();
    } else if (!beingPressed && wasPressed(keyPinIndex)) {
      //Serial.print("Released: ");
      //Serial.println(keyPinIndex);
      long x = ellapsedTimeFrom(lastReleasedKeyTimestamp);
      if (x > 50) {
        //Serial.print("Chorded: ");
        //Serial.print(pressedKeys);
        //Serial.print(", ");
        Serial.print(keymaps[mode]->pressedCharFor(pressedKeys));
      } else {
        //Serial.print("Not chorded: ");
        //Serial.println(x);
      }
      clearPressedKey(keyPinIndex);
      lastReleasedKeyTimestamp = millis();
    }
  }
}
