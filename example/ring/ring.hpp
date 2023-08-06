#pragma once
#include <cstdint>
#include <iosfwd>

namespace co_fsm::ring
{
    enum class automaton_id
    {
        ring_fsm
    };

    enum class event_id
    {
        start,
        clockwise,
        counter_clockwise,
    };

    using counter_type = std::uint16_t;
    using state_id = std::uint16_t;

    constexpr auto ready_state_id = state_id(~0U);

    std::ostream& operator<< (std::ostream& out, const automaton_id item);
    std::ostream& operator<< (std::ostream& out, const event_id item);
}
