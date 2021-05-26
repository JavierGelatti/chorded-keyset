#include "../keyset.h"
#include <Godmode.h>
#include <initializer_list>

const int keyPins[5] = {3, 2, 4, 6, 5};
const int modeLeds[3] = {15, 14, 16};
const int shiftLed = 10;
const int switchPins[2] = {9, 8};

class KeysetSimulator {
  public:
  KeysetSimulator() {
    this->keyset = new Keyset(keyPins, modeLeds, shiftLed, switchPins, [&](char pressedChar) {
      writtenText += pressedChar;
    });

    this->boardState = GODMODE();
    for (int i = 0; i < 5; i++) {
      this->boardState->digitalPin[keyPins[i]] = HIGH;
    }
  }
  ~KeysetSimulator() {
    delete this->keyset;
  }

  void start() {
    keyset->keysetSetup();
  }

  void press(int keyNumber) {
    boardState->digitalPin[keyPins[keyNumber-1]] = LOW;
    keyset->keysetLoop();
  }

  void release(int keyNumber) {
    boardState->digitalPin[keyPins[keyNumber-1]] = HIGH;
    keyset->keysetLoop();
  }

  void chord(std::initializer_list<int> keys) {
    for (auto key: keys) {
      press(key);
    }
    for (auto key: keys) {
      release(key);
    }
  }

  String getWrittenText() {
    String temp = writtenText;
    writtenText = "";
    return temp;
  }

  int getNumberOfModes() {
    return keyset->getNumberOfModes();
  }

  private:
  GodmodeState* boardState;
  Keyset* keyset;
  String writtenText = "";
};
