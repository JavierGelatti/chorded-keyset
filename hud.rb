require_relative 'command_processor'
require 'io/console'

display = ->(line) { STDOUT.write("#{line}\n") }
macro_runner = ->(macro) { `xdotool key #{macro}` }
command_processor = CommandProcessor.new(display, macro_runner)

STDOUT.sync = true

until STDIN.eof?
  command_processor.process_chord(STDIN.readpartial(1))
end
