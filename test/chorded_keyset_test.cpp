#include <ArduinoUnitTests.h>
#include "keyset_simulator.cpp"

unittest(setup_initializes_the_modes) {
  KeysetSimulator keyset;

  keyset.start();

  assertEqual(2, keyset.getNumberOfModes());
  assertEqual("", keyset.getWrittenText());
  assertEqual(false, keyset.shiftTurnedOn());
  assertEqual(1, keyset.currentMode());
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

unittest(shift_mode_works_for_one_character_only) {
  KeysetSimulator keyset;
  keyset.start();

  keyset.chord({ 1, 2, 3 });
  keyset.chord({ 5 });
  keyset.chord({ 5 });

  assertEqual("Uu", keyset.getWrittenText());
}

unittest(shift_led_turns_on_when_in_shift_mode) {
  KeysetSimulator keyset;
  keyset.start();

  keyset.chord({ 1, 2, 3 });

  assertEqual(true, keyset.shiftTurnedOn());
}

unittest(shift_led_turns_off_when_exit_shift_mode) {
  KeysetSimulator keyset;
  keyset.start();
  keyset.chord({ 1, 2, 3 });

  keyset.chord({ 1, 2, 3 });

  assertEqual(false, keyset.shiftTurnedOn());
}

unittest(can_change_mode) {
  KeysetSimulator keyset;
  keyset.start();

  keyset.chord({ 3, 4, 5 });

  assertEqual(2, keyset.currentMode());
}

unittest(writes_numbers_in_numeric_mode) {
  KeysetSimulator keyset;
  keyset.start();
  keyset.chord({ 3, 4, 5 });

  keyset.chord({ 1 });

  assertEqual("1", keyset.getWrittenText());
}

unittest(can_switch_to_log_mode) {
  KeysetSimulator keyset;
  keyset.start();
  keyset.chord({ 3, 4, 5 });
  keyset.chord({ 3, 4, 5 });

  keyset.chord({ 1 });

  assertEqual(3, keyset.currentMode());
  assertEqual("a", keyset.getLog());
  assertEqual("", keyset.getWrittenText());
}

unittest(switches_to_log_mode_from_command) {
  KeysetSimulator keyset;
  keyset.start();

  keyset.receiveCommand("log_mode");

  assertEqual(3, keyset.currentMode());
}

unittest(switches_to_normal_mode_from_command) {
  KeysetSimulator keyset;
  keyset.start();
  keyset.receiveCommand("log_mode");

  keyset.receiveCommand("normal_mode");

  assertEqual(1, keyset.currentMode());
}

unittest(ignores_unknown_commands) {
  KeysetSimulator keyset;
  keyset.start();

  keyset.receiveCommand("unknown_command");

  // The mode does not change
  assertEqual(1, keyset.currentMode());
}

unittest(reports_state_when_asked) {
  KeysetSimulator keyset;
  keyset.start();

  keyset.receiveCommand("query_state");

  auto response = keyset.lastResponse();

  assertEqual(0, response["mode"]);
}

unittest_main()