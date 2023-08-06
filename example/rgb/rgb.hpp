#pragma once
#include <iosfwd>

namespace co_fsm::rgb
{
    enum class automaton_id
    {
        blue_fsm,
        green_fsm,
        red_fsm,
    };

    enum class event_id
    {
        start_blink,
        blink_ready,
        hand_over,
    };

    enum class state_id
    {
        idle,
        active,
    };

    std::ostream& operator<< (std::ostream& out, const automaton_id item);
    std::ostream& operator<< (std::ostream& out, const event_id item);
    std::ostream& operator<< (std::ostream& out, const state_id item);
}
