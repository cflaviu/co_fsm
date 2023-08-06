#include "ring.hpp"
#include "ready_state_handler.hpp"
#include "ring_state_handler.hpp"

#include <array>
#include <iomanip>
#include <iostream>

namespace co_fsm::ring
{
    std::ostream& operator<< (std::ostream& out, const automaton_id item)
    {
        static const std::array<const char* const, 1U> texts {
            "ring_fsm",
        };

        out << texts[static_cast<int>(item)];
        return out;
    }

    std::ostream& operator<< (std::ostream& out, const event_id item)
    {
        static const std::array<const char* const, 3U> texts {
            "start",
            "clockwise",
            "counter_clockwise",
        };

        out << texts[static_cast<int>(item)];
        return out;
    }

    void setup(FSM& fsm, double& running_time_s, std::uint32_t& events_processed, const counter_type states_in_ring)
    {
        // Register the states in the ring of states.
        // They don't need individual names as they will be referred by the number 0...statesInRing-1
        for (counter_type i = 0U; i < states_in_ring; ++i)
            fsm << (coroutine(fsm, ring_state_handler(events_processed)).set_id(i));

        // Configure transitions clockwise from state i to state i+1
        // and counter clockwise from state i+1 to state i.
        for (counter_type i = 0U; i < states_in_ring - 1U; ++i)
        {
            fsm << FSM::transition(fsm.state_at(i).id(), event_id::clockwise, fsm.state_at(i + 1U).id());
            fsm << FSM::transition(fsm.state_at(i + 1U).id(), event_id::counter_clockwise, fsm.state_at(i).id());
        }

        // Register and name the ready state where the ring of states begin and end.
        fsm << (coroutine(fsm, ready_state_handler(running_time_s)) = ready_state_id);

        // Configure transitions from ready state to/from the first and last states of the ring
        // in both directions
        fsm << FSM::transition(ready_state_id, event_id::clockwise, fsm.state_at(0U).id())
            << FSM::transition(fsm.state_at(states_in_ring - 1U).id(), event_id::clockwise, ready_state_id)
            << FSM::transition(ready_state_id, event_id::counter_clockwise, fsm.state_at(states_in_ring - 1U).id())
            << FSM::transition(fsm.state_at(0U).id(), event_id::counter_clockwise, ready_state_id);

        // Launch the state coroutines and set the initial state.
        fsm.start().go_to(ready_state_id);
    }
}

int main()
{
    using namespace co_fsm::ring;
    FSM fsm {automaton_id::ring_fsm};
    double running_time_s {};          // Total running time in seconds
    std::uint32_t events_processed {}; // Number of times a state in the ring has received an event

#ifdef NDEBUG
    constexpr counter_type states_in_ring = 1023U;           // Number of states in the ring
    constexpr counter_type number_rounds_to_repeat = 10000U; // Number of times the ring will be cycled through.
#else
    // Reduced state cound and repeat count due sanitization overhead.
    constexpr counter_type states_in_ring = 127U;          // Number of states in the ring
    constexpr counter_type number_rounds_to_repeat = 100U; // Number of times the ring will be cycled through.
#endif

    setup(fsm, running_time_s, events_processed, states_in_ring);

    // Make the first event which will start the show.
    Event event {};
    event.set(event_id::start, number_rounds_to_repeat); // Cycle around the ring numRoundsToRepeat times.
    fsm.send_event(std::move(event));

    using std::cout;
    cout << fsm.id() << "' is suspended at state '" << fsm.state_id() << "'\n";
    cout << "Based on " << number_rounds_to_repeat << " rounds around the ring of " << states_in_ring << " states in "
         << std::setprecision(3) << std::fixed << running_time_s << " s, meaning " << (events_processed + number_rounds_to_repeat)
         << " events sent,\n"
         << "the speed of FSM's execution is " << std::fixed << (events_processed + number_rounds_to_repeat) / running_time_s
         << " state transitions/s\n";
    return 0;
}
