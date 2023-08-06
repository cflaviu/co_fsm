#pragma once
#include "morse.hpp"
#include <co_fsm/headers.hpp>
#include <variant>

namespace co_fsm::morse
{
    struct event: co_fsm::event_base<event_id>
    {
        using co_fsm::event_base<event_id>::set_id;

        std::variant<std::string, const std::string_view*, std::uint32_t> data {};

        const std::string& message() const { return std::get<std::string>(data); }
        std::string& message() { return std::get<std::string>(data); }

        const std::string_view& symbol() const { return *std::get<const std::string_view*>(data); }

        std::uint32_t beep_time_in_ms() const { return std::get<std::uint32_t>(data); }

        void set(const event_id id) noexcept
        {
            set_id(id);
            data = {};
        }

        void set_message(const event_id id, std::string message)
        {
            set_id(id);
            data = std::move(message);
        }

        void set_symbol(const event_id id, const std::string_view& symbol) noexcept
        {
            set_id(id);
            data = &symbol;
        }

        void set_beep_time(const event_id id, const std::uint32_t beep_time_in_ms) noexcept
        {
            set_id(id);
            data = beep_time_in_ms;
        }
    };
}
