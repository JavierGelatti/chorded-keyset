require_relative '../command_processor'

describe 'command processor' do
  subject(:processor) do
    CommandProcessor.new(display, macro_runner, commands)
  end

  let(:commands) do
    {
      "Delete Word" => "Control_L+Left Shift+Control_L+Right Delete"
    }
  end

  let(:display) { ->(text) { displayed_text << text } }
  let(:displayed_text) { [] }

  let(:macro_runner) { ->(macro) { run_macros << macro } }
  let(:run_macros) { [] }

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

  def time_travel_to(a_time)
    allow(Time).to receive(:now).and_return(a_time)
  end
end