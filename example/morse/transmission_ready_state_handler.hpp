#pragma once
#include "common.hpp"

namespace co_fsm::morse
{
    // FSM state coroutine which maps character strings into Morse symbols.
    class transmission_ready_state_handler
    {
    public:
        void operator() (const FSM& fsm, Event& event);

    private:
        static const std::unordered_map<char, std::string_view> symbol_map_;
        std::string message_ {};
        std::uint32_t symbols_sent_ {};
    };
}
