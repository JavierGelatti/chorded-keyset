require_relative '../command_processor'

describe 'command processor' do
  subject(:processor) do
    command_processor_for_commands(commands)
  end

  let(:commands) do
    {
      general: {
        "Delete Word" => "Control_L+Left Shift+Control_L+Right Delete"
      }
    }
  end

  let(:display) { ->(text) { displayed_text << text } }
  let(:displayed_text) { [] }

  let(:macro_runner) { ->(macro) { run_macros << macro } }
  let(:run_macros) { [] }

  let(:active_app) { 'random-app' }

  it 'gives feedback when processing a top-level command' do
    processor.process_chord("d")

    expect(displayed_text).to contain_exactly("Delete _")
  end

  it 'gives feedback and performs an action when processing a leaf command' do
    processor.process_chord("d")
    processor.process_chord("w")

    expect(displayed_text).to contain_exactly("Delete _", "Delete Word")
    expect(run_macros).to contain_exactly("Control_L+Left Shift+Control_L+Right Delete")
  end

  it 'resets its state when an unknown command is received' do
    processor.process_chord("d")
    processor.process_chord("unknown")
    processor.process_chord("d")

    expect(displayed_text).to contain_exactly("Delete _", "", "Delete _")
    expect(run_macros).to be_empty
  end

  it 'resets its state after 6 seconds of the last command' do
    t_0 = Time.new(2021, 5, 17, 1, 45, 0)
    time_travel_to(t_0)
    processor.process_chord("d")
    time_travel_to(t_0 + 6)

    processor.process_chord("w")

    expect(run_macros).to be_empty
  end

  it 'does not reset its state after 5 seconds of the last command' do
    t_0 = Time.new(2021, 5, 17, 1, 45, 0)
    time_travel_to(t_0)
    processor.process_chord("d")
    time_travel_to(t_0 + 5)

    processor.process_chord("w")

    expect(run_macros).not_to be_empty
  end

  it 'considers the timeout time differnece from the _last_ valid command' do
    t_0 = Time.new(2021, 5, 17, 1, 45, 0)
    time_travel_to(t_0)
    processor # Instantiate processor

    time_travel_to(t_0 + 10)
    processor.process_chord("d")
    processor.process_chord("w")

    expect(run_macros).not_to be_empty
  end

  it 'fails if more than one command has the same associated chord' do
    expect do
      command_processor_for_commands(
        general: {
          "Delete Word" => "d+w",
          "Do Work" => "d+o+w"
        }
      )
    end.to raise_error("The chord DW is associated with many commands: Delete Word, Do Work")
  end

  it 'fails if more than one command has the same chord prefix' do
    expect do
      command_processor_for_commands(
        general: {
          "Delete Word" => "d+w",
          "Do Work Once" => "d+w+o",
        }
      )
    end.to raise_error("The chord DW is associated with many commands: Delete Word, Do Work Once")
  end

  context 'when there are app-specific commands' do
    let(:commands) do
      {
        general: {
          "Only General" => "Ctrl+o+g",
          "Both General Specific" => "Ctrl+g",
        },
        app_specific: {
          "an_app" => {
            "Only Specific" => "Ctrl+o+s",
            "Both General Specific" => "Ctrl+s",
          }
        }
      }
    end

    context 'and the active app has commands' do
      let(:active_app) { 'an_app' }

      it 'respond to that app specific commands' do
        processor.process_chord("o")
        processor.process_chord("s")

        expect(displayed_text.last).to eq("Only Specific")
        expect(run_macros).to contain_exactly("Ctrl+o+s")
      end

      it 'still responds to other general commands' do
        processor.process_chord("o")
        processor.process_chord("g")

        expect(displayed_text.last).to eq("Only General")
        expect(run_macros).to contain_exactly("Ctrl+o+g")
      end

      it 'gives priority to specific commands if the command name is exactly the same' do
        processor.process_chord("b")
        processor.process_chord("g")
        processor.process_chord("s")

        expect(displayed_text.last).to eq("Both General Specific")
        expect(run_macros).to contain_exactly("Ctrl+s")
      end

      xit 'gives priority to specific commands if the app command chord matches a general command chord' do
        processor = command_processor_for_commands(
          general: {
            "gEneral Chord" => "Ctrl+g"
          },
          app_specific: {
            "an_app" => {
              "spEcific Chord" => "Ctrl+s"
            }
          }
        )

        processor.process_chord("e")
        processor.process_chord("c")

        expect(displayed_text.last).to eq("spEcific Chord")
        expect(run_macros).to contain_exactly("Ctrl+s")
      end

    end

    context 'and the active app has no commands' do
      let(:active_app) { 'another_app' }

      it 'responds to general commands' do
        processor.process_chord("b")
        processor.process_chord("g")
        processor.process_chord("s")

        expect(displayed_text.last).to eq("Both General Specific")
        expect(run_macros).to contain_exactly("Ctrl+g")
      end
    end

    it 'fails if more than one command has the same associated chord, including the app name in the error' do
      expect do
        command_processor_for_commands(
          app_specific: {
            "an_app" => {
              "A Chord" => "Ctrl+a",
              "Another Chord" => "Ctrl+b",
            }
          }
        )
      end.to raise_error("For an_app: The chord AC is associated with many commands: A Chord, Another Chord")
    end
  end

  def time_travel_to(a_time)
    allow(Time).to receive(:now).and_return(a_time)
  end

  def command_processor_for_commands(commands)
    CommandProcessor.for(display, macro_runner, -> { active_app }, commands.transform_keys(&:to_s))
  end
end