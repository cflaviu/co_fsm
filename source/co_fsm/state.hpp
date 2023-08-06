#pragma once
#ifndef PCH
    #include <coroutine>
    #include <sstream>
#endif

namespace co_fsm
{
    // It returns the type of coroutines which represent states.
    template <typename _Id>
    class state
    {
    public:
        using id_type = _Id;

        struct promise_type
        {
            using handle_type = std::coroutine_handle<promise_type>;

            struct initial_awaitable
            {
                promise_type* self {};
                constexpr bool await_ready() const noexcept { return {}; }
                void await_suspend(handle_type) const noexcept {}
                void await_resume() noexcept { self->is_started = true; } // The state was resumed from initial_suspend
            };

            initial_awaitable initial_suspend() noexcept { return {this}; }
            constexpr std::suspend_always final_suspend() noexcept { return {}; }
            state get_return_object() noexcept { return state(this); };
            void unhandled_exception() { throw; }
            void return_void()
            {
                // State coroutines must never return.
                std::ostringstream error_message {};
                error_message << "State coroutine '" << id << "' is not allowed to co_return.";
                throw std::runtime_error(error_message.str());
            }

            id_type id {};
            bool is_started {}; // false if the state is waiting at initial_suspend, true if the state has been resumed from the
                                // initial_suspend.
        };

        using handle_type = promise_type::handle_type;

        // A state is move-only
        state(const state&) = delete;
        state(state&& other) noexcept: handle_(std::exchange(other.handle_, nullptr)) {}
        ~state()
        {
            if (handle_)
                handle_.destroy();
        }

        // State identification.
        id_type id() const noexcept { return handle_.promise().id; }

        // It sets the state id.
        state&& set_id(const id_type id) noexcept
        {
            handle_.promise().id = id;
            return std::move(*this);
        }

        // It returns false if the state is still waiting in initial_suspend.
        // It returns true if the initial await has been resumed (typically by calling automaton::start()).
        bool is_started() const noexcept { return handle_.promise().is_started; }

        // It returns the handle to the state coroutine.
        const handle_type& handle() const noexcept { return handle_; }

        state& operator= (const state&) = delete;
        state& operator= (state&& other) noexcept
        {
            handle_ = std::exchange(other.handle_, nullptr);
            return *this;
        }

        // Alias for set_id.
        state&& operator= (const id_type id) noexcept { return std::move(set_id(id)); }

    private:
        explicit state(promise_type* promise) noexcept: handle_(handle_type::from_promise(*promise)) {}

        handle_type handle_;
    }; // State
}
