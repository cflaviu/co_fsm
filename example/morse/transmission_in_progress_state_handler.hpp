#pragma once
#include "common.hpp"

namespace co_fsm::morse
{
    // FSM state coroutine which controls the timings of dots and dashes.
    // It receives a string of dots and dashes and transmits them one by one.
    class transmission_in_progress_state_handler
    {
    public:
        transmission_in_progress_state_handler(const std::uint8_t speed_words_per_minute);

        void operator() (const FSM& fsm, Event& event);

    private:
        std::uint32_t dot_time_in_ms_;
        std::uint32_t dash_time_in_ms_;
        std::uint32_t signalsTransmitted_ {}; // Number of symbols transmitted so far.
        std::string_view symbol_ {};
    };
}
