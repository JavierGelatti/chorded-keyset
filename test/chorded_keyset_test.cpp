#include <ArduinoUnitTests.h>
#include "keyset_simulator.cpp"

unittest(setup_initializes_the_modes) {
  KeysetSimulator keyset;

  keyset.start();

  assertEqual(2, keyset.getNumberOfModes());
  assertEqual("", keyset.getWrittenText());
}

unittest(can_type_by_single_press) {
  KeysetSimulator keyset;
  keyset.start();

  keyset.press(1);
  keyset.release(1);

  assertEqual("a", keyset.getWrittenText());
}

unittest(can_type_chord_by_nesting_key_presses) {
  KeysetSimulator keyset;
  keyset.start();

  keyset.press(5);
  keyset.press(4);
  keyset.release(4);
  keyset.release(5);

  assertEqual("s", keyset.getWrittenText());
}

unittest(can_type_chord_by_intertwining_key_presses) {
  KeysetSimulator keyset;
  keyset.start();

  keyset.press(5);
  keyset.press(4);
  keyset.release(5);
  keyset.release(4);

  assertEqual("s", keyset.getWrittenText());
}

unittest(shift_mode_can_be_used_to_write_in_uppercase) {
  KeysetSimulator keyset;
  keyset.start();

  keyset.chord({ 1, 2, 3 });
  keyset.chord({ 5 });

  assertEqual("U", keyset.getWrittenText());
}

unittest_main()