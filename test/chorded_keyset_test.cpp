#include <ArduinoUnitTests.h>
#include <Godmode.h>
#include "../keyset.h"
#include <initializer_list>

const int keyPins[5] = {3,2,4,6,5};
const int modeLeds[] = {7, 8, 9};

class KeysetBoard {
  public:
  KeysetBoard() {
    this->keyset = new Keyset(keyPins, modeLeds);

    this->boardState = GODMODE();
    for (int i = 0; i < 5; i++) {
      this->boardState->digitalPin[keyPins[i]] = HIGH;
    }
  }
  ~KeysetBoard() {
    delete this->keyset;
  }

  void start() {
    keyset->keysetSetup();
  }

  void press(int keyNumber) {
    boardState->digitalPin[keyPins[keyNumber-1]] = LOW;
    keyset->keysetLoop([&](char pressedChar) {
      if (pressedChar != '\0') writtenText += pressedChar;
    });
  }

  void release(int keyNumber) {
    boardState->digitalPin[keyPins[keyNumber-1]] = HIGH;
    keyset->keysetLoop([&](char pressedChar) {
      if (pressedChar != '\0') writtenText += pressedChar;
    });
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

unittest(setup_initializes_the_modes) {
  KeysetBoard keyset;

  keyset.start();

  assertEqual(2, keyset.getNumberOfModes());
  assertEqual("", keyset.getWrittenText());
}

unittest(can_type_by_single_press) {
  KeysetBoard keyset;
  keyset.start();

  keyset.press(1);
  keyset.release(1);

  assertEqual("a", keyset.getWrittenText());
}

unittest(can_type_chord_by_nesting_key_presses) {
  KeysetBoard keyset;
  keyset.start();

  keyset.press(5);
  keyset.press(4);
  keyset.release(4);
  keyset.release(5);

  assertEqual("s", keyset.getWrittenText());
}

unittest(can_type_chord_by_intertwining_key_presses) {
  KeysetBoard keyset;
  keyset.start();

  keyset.press(5);
  keyset.press(4);
  keyset.release(5);
  keyset.release(4);

  assertEqual("s", keyset.getWrittenText());
}

unittest(shift_mode_can_be_used_to_write_in_uppercase) {
  KeysetBoard keyset;
  keyset.start();

  keyset.chord({1, 2, 3});
  keyset.chord({5});

  assertEqual("U", keyset.getWrittenText());
}

unittest_main()