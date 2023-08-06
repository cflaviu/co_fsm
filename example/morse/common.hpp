#pragma once
#include "event.hpp"

namespace co_fsm::morse
{
    using FSM = automaton<event, state<state_id>, automaton_id>;
    using Event = FSM::event_type;
    using State = FSM::state_type;
}
