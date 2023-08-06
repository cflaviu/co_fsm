#include "sound_on_state_handler.hpp"
#include "sound_controller.hpp"

#include <thread>

namespace co_fsm::morse
{
    void sound_on_state_handler::operator() (const FSM& fsm, Event& event)
    {
        if (event == event_id::do_beep)
        {
            // Beep for the given number of milliseconds
            sound_controller_.set(sound_controller::status::on);
            std::this_thread::sleep_for(std::chrono::milliseconds {event.beep_time_in_ms()});
            sound_controller_.set(sound_controller::status::off);

            // Recycle the BeebEvent into BeebDoneEvent.
            event.set(event_id::beep_done);
        }
        else // The event was not recognized.
        {
            std::ostringstream msg {};
            msg << "Unrecognized event '" << event.id() << "' received in state " << fsm.state_id();
            throw std::runtime_error(msg.str());
        }
    }
}
