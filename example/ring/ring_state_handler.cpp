#include "ring_state_handler.hpp"

namespace co_fsm::ring
{
    void ring_state_handler::operator() (const FSM& fsm, Event& event)
    {
        switch (event.id())
        {
            case event_id::clockwise:
            case event_id::counter_clockwise:
            {
                ++event_processed_count; // An expected event was received.
                break;
            }
            default:
            {
                std::ostringstream msg {};
                msg << "Unrecognized event '" << event.id() << "' received in state " << fsm.state_id();
                throw std::runtime_error(msg.str());
            }
        }
    }
}
