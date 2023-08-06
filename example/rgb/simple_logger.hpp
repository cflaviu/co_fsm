#pragma once
#include <iostream>
#include <mutex>

namespace co_fsm::rgb
{
    // Helper for state transition tracing
    struct simple_logger
    {
        enum state
        {
            off,
            on
        };

        state on_off_state = on; // Master switch. Off means print nothing.
        bool print_thread_id = false;
        void operator() (const auto /*fsm_id*/, const auto target_fsm_id, const auto fromState, const auto onEvent, const auto toState)
        {
            if (on_off_state == off)
                return;
            if (print_thread_id)
                atomic_print(target_fsm_id, " : event '", onEvent, "' from '", fromState, "' to '", toState, "', thread id = 0x", std::hex,
                             std::this_thread::get_id(), std::dec);
            else
                atomic_print(target_fsm_id, " : event '", onEvent, "' from '", fromState, "' to '", toState, "'");
        }

        template <class... Args>
        void atomic_print(Args&&... args)
        {
            std::lock_guard<std::mutex> lock(led_mutex);
            (std::cout << ... << args) << '\n';
        }
    };
}
