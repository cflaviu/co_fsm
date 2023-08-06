#include "rgb.hpp"
#include "common.hpp"
#include "simple_logger.hpp"

#include <array>

namespace co_fsm::rgb
{
    // FSM factories defined elsewhere.
    FSM& setup_red_fsm();
    FSM& setup_green_fsm();
    FSM& setup_blue_fsm();

    std::ostream& operator<< (std::ostream& out, const automaton_id item)
    {
        static const std::array<const char* const, 3U> texts {
            "blue_fsm",
            "green_fsm",
            "red_fsm",
        };

        out << texts[static_cast<int>(item)];
        return out;
    }

    std::ostream& operator<< (std::ostream& out, const event_id item)
    {
        static const std::array<const char* const, 3U> texts {
            "start_blink",
            "blink_ready",
            "hand_over",
        };

        out << texts[static_cast<int>(item)];
        return out;
    }

    std::ostream& operator<< (std::ostream& out, const state_id item)
    {
        static const std::array<const char* const, 2U> texts {
            "idle",
            "active",
        };

        out << texts[static_cast<int>(item)];
        return out;
    }

    using namespace std::literals::chrono_literals;

    std::mutex led_mutex;
}

int main()
{
    using namespace co_fsm::rgb;
    // Make FSMs for red, green and blue devices.
    // The factories are living in separate source files which are compiled independently.
    FSM& red_fsm = setup_red_fsm();
    FSM& green_fsm = setup_green_fsm();
    FSM& blue_fsm = setup_blue_fsm();

    // Connect the FSMs by setting inter-FSM transitions so that
    // the FSMs run cyclically in order red-->green-->blue.

    // When the red FSM sends a handover event, the green FSM will take over.
    red_fsm << FSM::transition(state_id::idle, event_id::hand_over, state_id::idle, &green_fsm);
    // When the green FSM sends a handover event, the blue FSM will take over.
    green_fsm << FSM::transition(state_id::idle, event_id::hand_over, state_id::idle, &blue_fsm);
    // When the blue FSM sends a handover event, the red FSM will take over.
    blue_fsm << FSM::transition(state_id::idle, event_id::hand_over, state_id::idle, &red_fsm);

    // Activate the FSMs and set their respective intial states.
    red_fsm.start().go_to(state_id::idle);
    green_fsm.start().go_to(state_id::idle);
    blue_fsm.start().go_to(state_id::idle);

    // Activate tracing. Change to "#if 1" to use live tracing.
#if 0
    simple_logger logger {};
    red_fsm.set_logger(logger);
    green_fsm.set_logger(logger);
    blue_fsm.set_logger(logger);
#endif

    // Start the action by sending a handover event to the given FSM.
    auto kick_off = [](std::stop_token stopToken, FSM* fsm)
    {
        Event e {};
        // The stop token of the thread piggy-backs to the fsm on the HandOver event.
        e.set(event_id::hand_over, stopToken);
        fsm->send_event(std::move(e));
    };

    // Tell which FSMs are running
    auto printActive = [&](std::string str)
    {
        simple_logger().atomic_print(str, " RED active = ", red_fsm.is_active(), ", GREEN active = ", green_fsm.is_active(),
                                     ", BLUE active = ", blue_fsm.is_active());
    };

    using std::cout;

    // Run the combined FSM sequentially 3 times, each time starting from a different state.
    // Note that stop_token requests the stop automatically when a jthread
    // is about to go out of scope.
    {
        cout << "---------------- Start the cycle with RED ----------------\n";
        std::jthread thread(kick_off, &red_fsm);
        std::this_thread::sleep_for(3000ms);  // Sleep, then request stop.
        printActive("Activity before stop:"); // Only 1 of 3 should be running
    }
    {
        cout << "---------------- Start the cycle with GREEN ---------------- \n";
        std::jthread thread(kick_off, &green_fsm);
        std::this_thread::sleep_for(3000ms);  // Sleep, then request stop.
        printActive("Activity before stop:"); // Only 1 of 3 should be running
    }
    {
        cout << "---------------- Start the cycle with BLUE ----------------\n";
        std::jthread thread(kick_off, &blue_fsm);
        std::this_thread::sleep_for(3000ms);  // Sleep, then request stop.
        printActive("Activity before stop:"); // Only 1 of 3 should be running
    }
    printActive("Activity after stop:"); // All 3 should be stopped.

    // Re-configure transitions so that each FSM "hands over" to itself instead of another FSM,
    // making the FSMs independent.
    red_fsm << FSM::transition(state_id::idle, event_id::hand_over, state_id::idle);
    green_fsm << FSM::transition(state_id::idle, event_id::hand_over, state_id::idle);
    blue_fsm << FSM::transition(state_id::idle, event_id::hand_over, state_id::idle);

    // Show also the thread id in the trace.
    // redFSM->simple_logger = greenFSM->simple_logger = blueFSM->simple_logger = simple_logger {.printThreadId = true};
    // Now the FSMs are independent and can run in parallel.
    {
        cout << "---------------- Run RED, GREEN, BLUE in parallel ----------------\n";
        std::jthread red_thread(kick_off, &red_fsm);
        std::jthread green_thread(kick_off, &green_fsm);
        std::jthread blue_thread(kick_off, &blue_fsm);
        std::this_thread::sleep_for(2s);
        printActive("All 3 are running in parallel:"); // All 3 should be active
    }
    printActive("All 3 have stopped:"); // All 3 should be inactive

    cout << "RED   fsm is suspended at state " << red_fsm.state_id() << '\n';
    cout << "GREEN fsm is suspended at state " << green_fsm.state_id() << '\n';
    cout << "BLUE  fsm is suspended at state " << blue_fsm.state_id() << '\n';
    return 0;
}
