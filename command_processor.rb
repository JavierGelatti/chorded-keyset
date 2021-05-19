require 'trie'

class CommandProcessor
  class << self
    def for(display, macro_runner, active_app_supplier, commands)
      general_commands = commands.fetch("general", {})
      app_specific_commands = commands.
        fetch("app_specific", {}).
        transform_values do |commands_just_for_app|
          general_commands.merge(commands_just_for_app)
        end

      new(
        active_app_supplier,
        FocusedCommandProcessor.new(display, macro_runner, chord_commands_from(general_commands)),
        app_specific_commands.map do |app, app_specific_commands|
          [app, FocusedCommandProcessor.new(display, macro_runner, chord_commands_from(app_specific_commands, app))]
        end.to_h
      )
    end

    private

    def chord_commands_from(command_definitions, app_name = nil)
      commands = command_definitions.map { |command_name, command_macro| KeysetCommand.new(command_name, command_macro) }

      assert_no_duplicated_chords_in(commands, app_name)

      commands.map { |command| [command.chords, command] }.to_h
    end

    def assert_no_duplicated_chords_in(commands, app_name)
      commands.map(&:chords).uniq.each do |chords|
        matching_commands = commands.select { |command| command.could_match_chords?(chords) }

        if matching_commands.size > 1
          raise "#{app_name && "For #{app_name}: "}The chord #{chords.upcase} is associated with many commands: #{commands.map(&:name).join(", ")}"
        end
      end
    end
  end

  def initialize(active_app_supplier, general_command_processor, app_specific_command_processors)
    @general_command_processor = general_command_processor
    @app_specific_command_processors = app_specific_command_processors
    @active_app_supplier = active_app_supplier
  end

  def process_chord(chorded_character)
    active_app = @active_app_supplier.call

    (@app_specific_command_processors[active_app] || @general_command_processor).process_chord(chorded_character)
  end
end

class KeysetCommand < Struct.new(:name, :macro)
  NIL = new("", "")

  def chords
    name.scan(/[[:upper:]]/).join.downcase
  end

  def could_match_chords?(an_input)
    chords.start_with?(an_input)
  end
end

class FocusedCommandProcessor
  MAX_SECONDS_DELAY = 5

  def initialize(display, macro_runner, chord_commands)
    @display = display
    @macro_runner = macro_runner
    @chord_commands = chord_commands
    @chorded_sequence = ""
    @last_chord_timestamp = Time.now
  end

  def process_chord(chorded_character)
    clear_chorded_sequence if Time.now - @last_chord_timestamp > MAX_SECONDS_DELAY
    @last_chord_timestamp = Time.now
    @chorded_sequence += chorded_character

    clear_chorded_sequence and return unless possible_command?

    if completed_command?
      @macro_runner.call(current_command)
      clear_chorded_sequence
    else
      # Accumulate chord
      @display.call("#{completed_words} _")
    end
  end

  private

  attr_reader :chord_sequences

  def commands_trie
    @commands_trie ||= trie_with(
      @chord_commands.values.map do |command|
        [command.chords, command.macro]
      end
    )
  end

  def trie_with(associations)
    trie = Trie.new
    associations.each do |key, value|
      trie.add(key, value)
    end
    trie
  end

  def possible_commands
    commands_trie.children(@chorded_sequence)
  end

  def possible_command?
    !possible_commands.empty?
  end

  def completed_command?
    !current_command.nil?
  end

  def current_command
    commands_trie.get(@chorded_sequence)
  end

  def completed_words
    first_command_name = @chord_commands.fetch(possible_commands.first, KeysetCommand::NIL).name
    completed_words = first_command_name.split(" ").take(@chorded_sequence.size).join(" ")
  end

  def clear_chorded_sequence
    @display.call(completed_words)
    @chorded_sequence = ""
  end
end