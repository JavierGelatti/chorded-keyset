require_relative '../command_processor'

describe 'command processor' do
  subject(:processor) do
    CommandProcessor.new(display, macro_runner)
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

    expect(displayed_text).to contain_exactly("Delete _", "Delete _")
  end
end