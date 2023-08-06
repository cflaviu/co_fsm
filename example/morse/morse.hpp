#pragma once
#include <iosfwd>

namespace co_fsm::morse
{
    enum class automaton_id
    {
        morse_fsm
    };

    enum class event_id
    {
        transmit_message,
        transmit_symbol,
        transmit_ready,
        do_beep,
        beep_done,
    };

    enum class state_id
    {
        transmission_ready,
        transmission_in_progress,
        sound_on,
    };

    std::ostream& operator<< (std::ostream& out, const automaton_id item);
    std::ostream& operator<< (std::ostream& out, const event_id item);
    std::ostream& operator<< (std::ostream& out, const state_id item);
}
