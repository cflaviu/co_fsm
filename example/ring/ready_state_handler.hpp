#pragma once
#include "common.hpp"

#include <chrono>

namespace co_fsm::ring
{
    // FSM state coroutine which cycles through the ring of states several times
    // after having received StartEvent and stores the average number of
    // state transitions per second in *transitionsPerSecond.
    class ready_state_handler
    {
    public:
        ready_state_handler(double& running_time_s): running_time_s(running_time_s) {}

        void operator() (const FSM& fsm, Event& event);

    private:
        enum class direction
        {
            clockwise,
            counter_clockwise
        };

        using clock = std::chrono::high_resolution_clock;
        using time_point = clock::time_point;

        time_point start_time_ {};
        time_point end_time_ {};
        double& running_time_s;
        counter_type rounds_left_ {};
        direction direction_ {};
    };
}
