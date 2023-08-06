#include "simple_logger.hpp"

#include <array>
#include <co_fsm/headers.hpp>
#include <iostream>

namespace co_fsm::ping_pong
{
    enum class automaton_id
    {
        ping_pong_fsm
    };

    enum class event_id
    {
        to_ping,
        to_pong,
    };

    enum class state_id
    {
        ping,
        pong,
    };

    std::ostream& operator<< (std::ostream& out, const automaton_id item)
    {
        static const std::array<const char* const, 1U> texts {
            "ping_pong_fsm",
        };

        out << texts[static_cast<int>(item)];
        return out;
    }

    std::ostream& operator<< (std::ostream& out, const event_id item)
    {
        static const std::array<const char* const, 2U> texts {
            "to_ping",
            "to_pong",
        };

        out << texts[static_cast<int>(item)];
        return out;
    }

    std::ostream& operator<< (std::ostream& out, const state_id item)
    {
        static const std::array<const char* const, 2U> texts {
            "ping",
            "pong",
        };

        out << texts[static_cast<int>(item)];
        return out;
    }

    struct event: co_fsm::event_base<event_id>
    {
        using co_fsm::event_base<event_id>::set_id;
        using counter_type = std::uint8_t;

        counter_type counter {};

        void set(const event_id event, const counter_type counter)
        {
            set_id(event);
            this->counter = counter;
        }
    };

    using FSM = automaton<event, state<state_id>, automaton_id>;
    using Event = FSM::event_type;
    using State = FSM::state_type;

    void ping_state_handler(const FSM& fsm, Event& event)
    {
        if (event == event_id::to_ping)
        {
            if (event.counter != 0U) // Send to_pong if the counter is still positive
                // Re-construct the event from Ping to Pong
                event.set(event_id::to_pong, event.counter - 1U);
            else // Send an empty event to suspend the FSM
                event.invalidate();
        }
        else // The event was not recognized.
        {
            std::ostringstream msg {};
            msg << "Unrecognized event '" << event.id() << "' received in state " << fsm.state_id();
            throw std::runtime_error(msg.str());
        }
    }

    void pong_state_handler(const FSM& fsm, Event& event)
    {
        if (event == event_id::to_pong)
        {
            if (event.counter != 0U) // Send to_pong if the counter is still positive
                // Re-construct the event from Pong to Ping
                event.set(event_id::to_ping, event.counter - 1U);
            else // Send an empty event to suspend the FSM
                event.invalidate();
        }
        else // The event was not recognized.
        {
            std::ostringstream msg {};
            msg << "Unrecognized event '" << event.id() << "' received in state " << fsm.state_id();
            throw std::runtime_error(msg.str());
        }
    }

    /* Creates the states and sets the transition table like so:
       [ ping]  --- to_pong ---> [ pong]
       [State] <--- to_ping ---  [State]
    */
    void setup(FSM& fsm)
    {
        // Make and name the states
        fsm << (coroutine(fsm, ping_state_handler).set_id(state_id::ping)) << (coroutine(fsm, pong_state_handler).set_id(state_id::pong));
        // using namespace co_fsm;
        fsm << FSM::transition(state_id::ping, event_id::to_pong, state_id::pong)
            << FSM::transition(state_id::pong, event_id::to_ping, state_id::ping);

        // List the states.
        using std::cout;
        cout << '\'' << fsm.id() << "' has " << fsm.state_count() << " states.\n";
        cout << "The states are:\n";
        for (auto i = 0U; i < fsm.state_count(); ++i)
            cout << "  (" << i << ") " << fsm.state_at(i).id() << '\n';

        // List the transitions
        cout << "The transitions are:\n";
        const auto transitions = fsm.get_transitions();
        for (const auto& transition: transitions) // tr is an array of 3 string_views (from, event, to)
            cout << "  {" << transition.from << ", " << transition.event << "} --> " << transition.to << '\n';

        // Log the events to std:.cerr stream.
        // fsm.set_logger(simple_logger<FSM> {std::cerr});
        // fsm.logger_ = simple_logger<FSM> {std::cerr};

        fsm.start();
    }
}

int main()
{
    using namespace co_fsm::ping_pong;
    FSM fsm {automaton_id::ping_pong_fsm};

    // Create the states and the transition table and start the state coroutines
    setup(fsm);

    // Make the first event which starts the show
    Event event {};

    // Set the intial state as Ping and sent the first event to it.
    // Now the ping-pong-loop will run 3 times after which the FSM will suspend.
    using std::cout;
    cout << "\n1. Running...\n";
    event.set(event_id::to_ping, 3U);                       // Do the ping<->pong 3 times.
    fsm.go_to(state_id::ping).send_event(std::move(event)); // Send to_ping to ping state.

    // Now we should be back at Ping state.
    cout << fsm.id() << " suspended at state " << fsm.state_id() << '\n';

    // Do it again but this time start from Pong state.
    cout << "\n2. Running...\n";
    event.set(event_id::to_pong, 3U);                       // Do the ping<->pong 3 times.
    fsm.go_to(state_id::pong).send_event(std::move(event)); // Send to_pong to pong state.

    // Now we should be back at Pong state.
    cout << fsm.id() << " suspended at state " << fsm.state_id() << '\n';
    return 0;
}
