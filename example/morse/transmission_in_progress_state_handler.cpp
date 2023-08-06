#include "transmission_in_progress_state_handler.hpp"

#include <chrono>
#include <iostream>
#include <thread>

namespace co_fsm::morse
{
    transmission_in_progress_state_handler::transmission_in_progress_state_handler(const std::uint8_t speed_words_per_minute):
        dot_time_in_ms_(1200U / std::clamp(1U, 1200U, static_cast<std::uint32_t>(speed_words_per_minute))),
        dash_time_in_ms_(3U * dot_time_in_ms_)
    {
    }

    void transmission_in_progress_state_handler::operator() (const FSM& fsm, Event& event)
    {
        switch (event.id())
        {
            case event_id::transmit_symbol:
            {
                // Start transmission of a new symbol which consists of signals (i.e. dots and dashes.)
                // Get a pointer to a string_view of signals with operator>> and return a reference to the string_view.
                symbol_ = event.symbol();
                signalsTransmitted_ = {};
                break;
            }
            case event_id::beep_done:
            {
                // A signal has been transmitted. Insert a gap between dots and dashes.
                std::this_thread::sleep_for(std::chrono::milliseconds {dot_time_in_ms_});
                break;
            }
            default: // The event was not recognized.
            {
                std::ostringstream msg {};
                msg << "Unrecognized event '" << event.id() << "' received in state " << fsm.state_id();
                throw std::runtime_error(msg.str());
            }
        }

        // Transmit the next signal if any left in the symbol
        if (signalsTransmitted_ < symbol_.size())
        {
            const char signal = symbol_[signalsTransmitted_++];
            std::cout << signalsTransmitted_ << " = " << signal << '\n';
            switch (signal)
            {
                case '.': // Transmit dot
                {
                    event.set_beep_time(event_id::do_beep, dot_time_in_ms_);
                    break;
                }
                case '-': // Transmit dash
                {
                    event.set_beep_time(event_id::do_beep, dash_time_in_ms_);
                    break;
                }
                case ' ':
                {
                    // Gap between words is 7 dots.
                    // The gap between words, if present, is assumed to be the last signal of the symbol.
                    signalsTransmitted_ = symbol_.size();
                    std::this_thread::sleep_for(std::chrono::milliseconds {7U * dot_time_in_ms_});
                    event.set(event_id::transmit_ready); // construct(event_id::transmission_ready);
                    break;
                }
                default:
                {
                    // Unexpected signal.
                    std::ostringstream message {};
                    message << "Unexpected signal received " << int(signal);
                    throw std::runtime_error(message.str());
                }
            }
        }
        else
        { // The entire symbol has been transmitted. Complete symbol gap of 1+2 dot times.
            std::this_thread::sleep_for(std::chrono::milliseconds {2U * dot_time_in_ms_});
            event.set(event_id::transmit_ready);
        }
    }
}
