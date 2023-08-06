#include "ready_state_handler.hpp"

#include <chrono>

namespace co_fsm::ring
{
    void ready_state_handler::operator() (const FSM& fsm, Event& event)
    {
        switch (event.id())
        {
            case event_id::start: // Start the measurement
            {
                rounds_left_ = std::max<counter_type>(event.round_counter, 1U);
                start_time_ = clock::now();
                break;
            }
            case event_id::clockwise: // One round of the ring of states done
            {
                // The next round will circulate at the opposite direction
                direction_ = direction::counter_clockwise;
                break;
            }
            case event_id::counter_clockwise: // One round of the ring of states done
            {
                // The next round will circulate at the opposite direction
                direction_ = direction::clockwise;
                break;
            }
            default:
            {
                // The event was not recognized.
                std::ostringstream msg {};
                msg << "Unrecognized event '" << event.id() << "' received in state " << fsm.state_id();
                throw std::runtime_error(msg.str());
            }
        }

        if (rounds_left_ > 0U) // Start another round of states in the given direction
        {
            --rounds_left_;
            event.set_id(direction_ == direction::clockwise ? event_id::clockwise : event_id::counter_clockwise);
        }
        else // The required number of rounds done.
        {
            end_time_ = clock::now();
            const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end_time_ - start_time_);
            running_time_s += diff.count() / 1000.f;
            event.invalidate(); // Suspend the FSM by sending an empty event
        }
    }
}
