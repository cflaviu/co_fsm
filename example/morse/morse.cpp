#include "simple_logger.hpp"
#include "sound_controller.hpp"
#include "sound_on_state_handler.hpp"
#include "transmission_in_progress_state_handler.hpp"
#include "transmission_ready_state_handler.hpp"

#include <array>
#include <iostream>

namespace co_fsm::morse
{
    std::ostream& operator<< (std::ostream& out, const automaton_id item)
    {
        static const std::array<const char* const, 1U> texts {
            "morse_fsm",
        };

        out << texts[static_cast<int>(item)];
        return out;
    }

    std::ostream& operator<< (std::ostream& out, const event_id item)
    {
        static const std::array<const char* const, 5U> texts {
            "transmit_message", "transmit_symbol", "transmit_ready", "do_beep", "beep_done",
        };

        out << texts[static_cast<int>(item)];
        return out;
    }

    std::ostream& operator<< (std::ostream& out, const state_id item)
    {
        static const std::array<const char* const, 3U> texts {
            "transmission_ready",
            "transmission_in_progress",
            "sound_on",
        };

        out << texts[static_cast<int>(item)];
        return out;
    }

    void setup(FSM& fsm, sound_controller& sound_controller)
    {
        constexpr std::uint8_t wordsPerMinute = 12U; // Approximate transmission speed in words per minute.

        // Register the state coroutines and give them ids.
        fsm << (coroutine(fsm, transmission_ready_state_handler {}).set_id(state_id::transmission_ready))
            << (coroutine(fsm, transmission_in_progress_state_handler {wordsPerMinute}).set_id(state_id::transmission_in_progress))
            << (coroutine(fsm, sound_on_state_handler {sound_controller}).set_id(state_id::sound_on));

        // Configure the transition table in format ("From State", "Event", "To State")
        // Example: event_id::transmit_symbol sent from state_id::transmission_ready state goes to
        // state_id::transmission_in_progress state
        fsm << FSM::transition(state_id::transmission_ready, event_id::transmit_symbol, state_id::transmission_in_progress)
            << FSM::transition(state_id::transmission_in_progress, event_id::transmit_ready, state_id::transmission_ready)
            << FSM::transition(state_id::transmission_in_progress, event_id::do_beep, state_id::sound_on)
            << FSM::transition(state_id::sound_on, event_id::beep_done, state_id::transmission_in_progress);

        // Change to "#if 1" to use live tracing.
        // You can filter the log messages out by running "sudo ./fsm-example-morse 2> /dev/null"
        // The sudo is needed only if LEDs are used (i.e. built with "make linux")
#if 0
    fsm.set_logger(simple_logger<FSM> {std::cerr});
#endif

        // Launch the state coroutines and set the initial state.
        fsm.start().go_to(state_id::transmission_ready);
    }
}

int main()
{
    using namespace co_fsm::morse;
    sound_controller sound_controller {};
    FSM fsm {automaton_id::morse_fsm};
    setup(fsm, sound_controller);

    // Make the first event which will start the show.
    Event event {};

    // Send these sentences
    const std::vector<std::string> messages {"Hello World ", "SOS SOS ", "Wikipedia the free encyclopedia"};
    for (const auto& message: messages)
    {
        std::cout << "Message = '" << message << "'\n";
        event.set_message(event_id::transmit_message, message);
        fsm.send_event(std::move(event));
    }

    std::cout << "\n'" << fsm.id() << "' is suspended at state '" << fsm.state_id() << "'\n";
    return 0;
}
