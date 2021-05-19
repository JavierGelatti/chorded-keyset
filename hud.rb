require_relative 'command_processor'
require 'io/console'
require 'yaml'

commands = YAML::load(File.open("chords.yml"))

display = ->(line) { STDOUT.write("#{line}\n") }
macro_runner = ->(macro) { `xdotool key #{macro}` }
active_app = -> {
  /^WM_CLASS\(STRING\) = "(.*)", "(.*)"$/.
    match(`xprop -id $(xdotool getwindowfocus) WM_CLASS`)[2]
}
command_processor = CommandProcessor.for(display, macro_runner, active_app, commands)

STDOUT.sync = true

until STDIN.eof?
  command_processor.process_chord(STDIN.readpartial(1))
end
