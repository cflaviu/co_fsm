#include "transmission_ready_state_handler.hpp"

#include <iostream>
#include <sstream>

namespace co_fsm::morse
{
    const std::unordered_map<char, std::string_view> transmission_ready_state_handler::symbol_map_ // Map characters to morse codes
        {
            {' ', " "},     {'A', ".-"},    {'B', "-..."},  {'C', "-.-."},  {'D', "-.."},   {'E', "."},     {'F', "..-."},  {'G', "--."},
            {'H', "...."},  {'I', ".."},    {'J', ".---"},  {'K', "-.-"},   {'L', ".-.."},  {'M', "--"},    {'N', "-."},    {'O', "---"},
            {'P', ".--."},  {'Q', "--.-"},  {'R', ".-."},   {'S', "..."},   {'T', "-"},     {'U', "..-"},   {'V', "...-"},  {'W', ".--"},
            {'X', "-..-"},  {'Y', "-.--"},  {'Z', "--.."},  {'1', ".----"}, {'2', "..---"}, {'3', "...--"}, {'4', "....-"}, {'5', "....."},
            {'6', "-...."}, {'7', "--..."}, {'8', "---.."}, {'9', "----."}, {'0', "-----"},
        };

    void transmission_ready_state_handler::operator() (const FSM& fsm, Event& event)
    {
        switch (event.id())
        {
            case event_id::transmit_message:
            {
                // Take the string from the event data and store.
                message_ = std::move(event.message());
                symbols_sent_ = {};
                break;
            }
            case event_id::transmit_ready:
            {
                // A symbol has been transmitted. Do the next one, if any.
                if (symbols_sent_ == message_.size()) // All symbols are sent.
                    event.invalidate();               // Suspend the FSM by sending an empty event.
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

        if (symbols_sent_ < message_.size())
        {
            const char upper_case_symbol = std::toupper(message_[symbols_sent_]);
            const char symbol = symbol_map_.contains(upper_case_symbol) ? upper_case_symbol : ' ';
            std::cout << "--> '" << symbol << "'\n";
            event.set_symbol(event_id::transmit_symbol, symbol_map_.at(symbol));
            ++symbols_sent_;
        }
    }
}
