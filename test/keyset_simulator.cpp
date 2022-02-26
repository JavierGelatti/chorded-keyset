#include "../keyset.h"
#include <ArduinoJson.h>
#include "../json_utils.h"
#include <Godmode.h>
#include <cassert>
#include <initializer_list>

const int keyPins[5] = {3, 2, 4, 6, 5};
const int modeLeds[3] = {15, 14, 16};
const int shiftLed = 10;

class KeysetSimulator {
  public:
  KeysetSimulator() {
    this->keyset = new Keyset(
      keyPins, modeLeds, shiftLed,
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
    int mode = -1;
    for (int i = 0; i < 3; i++) {
      if (boardState->digitalPin[modeLeds[i]] == HIGH) {
        assert(("More than one mode led is turned on", mode == -1));
        mode = i + 1;
      }
    }

    assert(("No mode led is turned on", mode != -1));
    return mode;
  }

  String getLog() {
    return eventLog;
  }

  void receiveCommand(string commandName) {
    StaticJsonDocument<200> commandDocument;
    commandDocument["command"] = commandName;

    boardState->serialPort[0].dataIn = serializeJson(commandDocument) + "\n";

    keyset->keysetLoop();
  }

  StaticJsonDocument<200> lastResponse() {
    StaticJsonDocument<200> responseDocument;

    String sentData = boardState->serialPort[0].dataOut;
    assert(("No data was sent!", sentData != ""));
    assert(("Sent data has missing ending newline", sentData.endsWith("\n")));

    String sentDataWithoutEndingNewline = sentData.substring(0, sentData.length() - 1);
    String lastResponseData = sentData.substring(sentDataWithoutEndingNewline.lastIndexOf("/n") + 1);

    DeserializationError error = deserializeJson(responseDocument, lastResponseData);
    assert(("Sent data is not JSON!", !error));

    return responseDocument;
  }

  private:
  GodmodeState* boardState;
  Keyset* keyset;
  String writtenText = "";
  String eventLog = "";
};
