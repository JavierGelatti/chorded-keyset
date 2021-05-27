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
    this->keyset = new Keyset(
      keyPins, modeLeds, shiftLed, switchPins,
      [&](char pressedChar) {
        writtenText += pressedChar;
      },
      [&](const char* event) {
        if (eventLog.length() != 0) eventLog += "|";
        eventLog += String(event);
      }
    );

    this->boardState = GODMODE();
    for (int i = 0; i < numKeys; i++) {
      this->boardState->digitalPin[keyPins[i]] = HIGH;
    }

    for (int i = 0; i < numSwitch; i++) {
      this->boardState->digitalPin[switchPins[i]] = HIGH;
    }
  }
  ~KeysetSimulator() {
    delete this->keyset;
  }

  void start() {
    keyset->keysetSetup();
    keyset->keysetLoop();
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

  bool shiftTurnedOn() {
    return boardState->digitalPin[shiftLed] == HIGH;
  }

  int currentMode() {
    int mode = 0;
    for (int i = 0; i < 3; i++) {
      mode += boardState->digitalPin[modeLeds[i]] << i;
    }
    return mode;
  }

  void turnOnRightHandMode() {
    boardState->digitalPin[switchPins[0]] = LOW;
  }

  bool inLeftHandMode() {
    return boardState->digitalPin[switchPins[0]] == HIGH;
  }

  String getLog() {
    return eventLog;
  }

  private:
  GodmodeState* boardState;
  Keyset* keyset;
  String writtenText = "";
  String eventLog = "";
};
