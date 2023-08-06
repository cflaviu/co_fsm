#pragma once
#include "rgb.hpp"
#include <co_fsm/headers.hpp>
#include <stop_token>
#include <variant>

namespace co_fsm::rgb
{
    using blink_time_ms_t = std::uint16_t;

    class event: public co_fsm::event_base<event_id>
    {
    public:
        const std::stop_token& stop_token() const noexcept { return std::get<std::stop_token>(data); }
        std::stop_token& stop_token() noexcept { return std::get<std::stop_token>(data); }

        blink_time_ms_t blink_time_in_ms() const noexcept { return std::get<blink_time_ms_t>(data); }

        void set(const event_id id) noexcept
        {
            set_id(id);
            data = {};
        }

        void set(const event_id id, std::stop_token stop_token) noexcept
        {
            set_id(id);
            data = std::move(stop_token);
        }

        void set(const event_id id, const blink_time_ms_t blink_time_ms) noexcept
        {
            set_id(id);
            data = blink_time_ms;
        }

    private:
        std::variant<std::monostate, std::stop_token, blink_time_ms_t> data {};
    };
}
