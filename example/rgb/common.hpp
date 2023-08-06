#pragma once
#include "led_control.hpp"
#include "my_event.hpp"

namespace co_fsm::rgb
{
    using FSM = automaton<event, state<state_id>, automaton_id>;
    using Event = FSM::event_type;
    using State = FSM::state_type;
}
