#pragma once
#include "common.hpp"

#include <chrono>

namespace co_fsm::ring
{
    // A state on the ring of states.
    // Passes either clockwise or counter-clockwise token event to the next state on the ring.
    class ring_state_handler
    {
    public:
        ring_state_handler(std::uint32_t& event_processed_count): event_processed_count(event_processed_count) {}

        void operator() (const FSM& fsm, Event& event);

    private:
        std::uint32_t& event_processed_count;
    };
}
