require 'trie'

class CommandProcessor
  MAX_SECONDS_DELAY = 5

  def initialize(display, macro_runner, commands)
    @display = display
    @macro_runner = macro_runner
    @chorded_sequence = ""
    @last_chord_timestamp = Time.now
    @commands = commands
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

  def chord_sequences
    @chord_sequences ||= @commands.keys.map do |command_name|
      [command_name, command_name.scan(/[[:upper:]]/).join.downcase]
    end.to_h
  end

  def command_names
    @command_names ||= @commands.keys.map do |command_name|
      [chord_sequences[command_name], command_name]
    end.to_h
  end

  def commands_trie
    @commands_trie ||= trie_with(
      @commands.keys.map do |command_name|
        [chord_sequences[command_name], @commands[command_name]]
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
    first_command_name = command_names.fetch(possible_commands.first, "")
    completed_words = first_command_name.split(" ").take(@chorded_sequence.size).join(" ")
  end

  def clear_chorded_sequence
    @display.call(completed_words)
    @chorded_sequence = ""
  end
end