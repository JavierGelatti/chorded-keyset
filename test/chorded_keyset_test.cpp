#include <ArduinoUnitTests.h>
#include <Godmode.h>
#include "../keyset.h"

const int keyPins[5] = {3,2,4,6,5};
const int modeLeds[] = {7, 8, 9};

Keyset keyset(keyPins, modeLeds);
//this->keyPins[numKeys] = {2, 3, 4, 5, 6};
//this->keyPins[numKeys] = {3,2,4,6,5};
//this->keyPins[numKeys] = {5,6,4,2,3};

GodmodeState* initialState() {
  auto state = GODMODE();
  for (int i = 0; i < 5; i++) {
    state->digitalPin[keyPins[i]] = HIGH;
  }
  return state;
}

String writtenText = "";

void press(int keyIndex, GodmodeState* state) {
  state->digitalPin[keyPins[keyIndex]] = LOW;
  keyset.keysetLoop([&](char pressedChar) {
    if (pressedChar != '\0') writtenText += pressedChar;
  });
}

void release(int keyIndex, GodmodeState* state) {
  state->digitalPin[keyPins[keyIndex]] = HIGH;
  keyset.keysetLoop([&](char pressedChar) {
    if (pressedChar != '\0') writtenText += pressedChar;
  });
}

unittest(chorded_keyset) {
  // setup_initializes_the_modes
  auto state = initialState();

  keyset.keysetSetup();

  assertEqual(2, keyset.getNumberOfModes());

  assertEqual("", writtenText);


  press(0, state);
  release(0, state);

  assertEqual("u", writtenText);
  writtenText = "";


  press(0, state);
  press(1, state);
  release(0, state);
  release(1, state);

  assertEqual("s", writtenText);
  writtenText = "";


  press(0, state);
  press(1, state);
  release(1, state);
  release(0, state);

  assertEqual("s", writtenText);
  writtenText = "";


  press(4, state);
  press(3, state);
  press(2, state);
  release(4, state);
  release(3, state);
  release(2, state);

  press(0, state);
  release(0, state);

  assertEqual("U", writtenText);
  writtenText = "";

  //
  // pin history is queued in case we want to analyze it later.
  // we expect 6 values in that queue (5 that we set plus one
  // initial value), which we'll hard-code here for convenience.
  // (we'll actually assert those 6 values in the next block)
//   assertEqual(6, state->digitalPin[1].queueSize());
//   bool expected[6] = {LOW, HIGH, LOW, LOW, HIGH, HIGH};
//   bool actual[6];

//   // convert history queue into an array so we can verify it.
//   // while we're at it, check that we received the amount of
//   // elements that we expected.
//   int numMoved = state->digitalPin[myPin].toArray(actual, 6);
//   assertEqual(6, numMoved);

//   // verify each element
//   for (int i = 0; i < 6; ++i) {
//     assertEqual(expected[i], actual[i]);
//   }
}

unittest_main()