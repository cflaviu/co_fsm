#pragma once
#include "common.hpp"

namespace co_fsm::morse
{
    class sound_controller;

    // FSM state coroutine which sets the sound on
    // and keeps it on for the time given in BeebEvent.
    class sound_on_state_handler
    {
    public:
        sound_on_state_handler(sound_controller& sound_controller): sound_controller_(sound_controller) {}

        void operator() (const FSM& fsm, Event& event);

    private:
        sound_controller& sound_controller_;
    };
}
