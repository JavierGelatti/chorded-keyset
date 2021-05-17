require_relative 'command_processor'
require 'io/console'

commands = {
  "Delete Word" => "Control_L+Left Shift+Control_L+Right Delete",
}

display = ->(line) { STDOUT.write("#{line}\n") }
macro_runner = ->(macro) { `xdotool key #{macro}` }
command_processor = CommandProcessor.new(display, macro_runner, commands)

STDOUT.sync = true

until STDIN.eof?
  command_processor.process_chord(STDIN.readpartial(1))
end
