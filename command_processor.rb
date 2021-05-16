class CommandProcessor

    def initialize(display, macro_runner)
        @display = display
        @macro_runner = macro_runner
        @chorded_sequence = ""
    end

    def process_chord(chorded_character)
        @chorded_sequence += chorded_character
        case @chorded_sequence
        when "d"
            @display.call("Delete _")
        when "dw"
            @display.call("Delete Word")
            @chorded_sequence = ""
            @macro_runner.call("Control_L+Left Shift+Control_L+Right Delete")
        else
            @chorded_sequence = ""
        end
    end


end