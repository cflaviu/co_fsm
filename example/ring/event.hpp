#pragma once
#include "ring.hpp"
#include <co_fsm/headers.hpp>

namespace co_fsm::ring
{
    struct event: co_fsm::event_base<event_id>
    {
        using co_fsm::event_base<event_id>::set_id;

        counter_type round_counter {};

        void set(const event_id event, const counter_type round_counter)
        {
            set_id(event);
            this->round_counter = round_counter;
        }
    };
}
