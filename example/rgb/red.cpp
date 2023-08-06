#include "common.hpp"

namespace co_fsm::rgb
{
    using std::cout;

    class active_state_handler
    {
        led_control led_control_ {"red"};

    public:
        void operator() (const FSM& fsm, Event& event)
        {
            if (event == event_id::start_blink)
            { // Keep the LED on for the given time in milliseconds.
                led_control_.set(led_control::Status::On);
                std::this_thread::sleep_for(std::chrono::milliseconds {event.blink_time_in_ms()});
                led_control_.set(led_control::Status::Off);
                // Recycle the event from "StartBlink" to "BlinkReady".
                event.set(event_id::blink_ready);
            }
            else
            { // The event was not recognized.
                std::ostringstream msg {};
                msg << "Unrecognized event '" << event.id() << "' received in state " << fsm.state_id();
                throw std::runtime_error(msg.str());
            }
        }
    };

    class idle_state_handler
    {
        std::stop_token stopToken {};
        std::uint8_t blinks_left {};

    public:
        void operator() (const FSM& fsm, Event& event)
        {
            constexpr std::uint16_t blink_time_ms = 250U; // Time of a single blink in milliseconds
            constexpr std::uint8_t number_of_blinks = 2U; // Number of blinks to be done before handing over to the next FSM.

            switch (event.id())
            {
                case event_id::hand_over: // This FSM gets control from another FSM
                {
                    stopToken = std::move(event.stop_token());
                    blinks_left = number_of_blinks;                  // Do this many blinks before handing over to another FSM
                    event.set(event_id::start_blink, blink_time_ms); // blink_time_ms piggybacks on "StartBlinkEvent"
                    break;
                }
                case event_id::blink_ready: // A blink is ready.
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(blink_time_ms));
                    if (stopToken.stop_requested())
                        event.invalidate(); // Send an empty event. It will suspend the FSM.
                    else if (blinks_left && (--blinks_left) > 0)
                    { // Do more blinks?
                        event.set(event_id::start_blink, blink_time_ms);
                    }
                    else
                    { // No more blinks, hand over to the next FSM by sending "HandOverEvent".
                        event.set(event_id::hand_over, std::move(stopToken));
                    }

                    break;
                }
                default:
                { // The event was not recognized.
                    std::ostringstream msg {};
                    msg << "Unrecognized event '" << event.id() << "' received in state " << fsm.state_id();
                    throw std::runtime_error(msg.str());
                }
            }
        }
    };

    FSM& setup_red_fsm()
    {
        static FSM fsm {automaton_id::red_fsm};
        // Register and name the states.
        fsm << (coroutine(fsm, idle_state_handler {}).set_id(state_id::idle))
            << (coroutine(fsm, active_state_handler {}).set_id(state_id::active));

        // Configure the transition table:
        //   When BlinkReady event is sent from Active state, go to Idle state.
        fsm << FSM::transition(state_id::active, event_id::blink_ready, state_id::idle)
            //   When StartBlink event is sent from Idle state, go to Active state.
            << FSM::transition(state_id::idle, event_id::start_blink, state_id::active)
            //   HandOver event (which contains stop token) is sent to self
            << FSM::transition(state_id::idle, event_id::hand_over, state_id::idle);

        return fsm;
    }
}
