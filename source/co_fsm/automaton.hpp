#pragma once
#ifndef PCH
    #include <atomic>
    #include <cassert>
    #include <coroutine>
    #include <functional>
    #include <optional>
    #include <source_location>
    #include <sstream>
#endif

namespace co_fsm
{
    // Default implementation of pair "state handle - event id" and of default (xor) hash of that pair.
    template <typename _State_handle_type, typename _Event_id_type>
    struct default_state_handle_event_id_pair: std::pair<_State_handle_type, _Event_id_type>
    {
        using base = std::pair<_State_handle_type, _Event_id_type>;
        using state_handle_type = _State_handle_type;
        using event_id_type = _Event_id_type;

        using base::base;

        // Hash {state, event} - pair
        struct hash
        {
            static auto handle_hash(const state_handle_type& handle) noexcept { return std::hash<void*>()(handle.address()); }
            static auto event_id_hash(const event_id_type id) noexcept { return std::hash<event_id_type>()(id); }

            std::size_t operator() (const default_state_handle_event_id_pair& pair) const noexcept
            {
                // Note: you could possibly do better than xor. See
                // https://stackoverflow.com/questions/5889238/why-is-xor-the-default-way-to-combine-hashes
                return handle_hash(pair.first) ^ event_id_hash(pair.second);
            }
        };
    };

    // Finite State Machine class.
    // default_state_handle_event_id_pair parameter allows the customization of "state handle - event id" pair and their hashing.
    template <typename _Event, typename _State, typename _Id = std::uint8_t,
              template <typename...> class _State_handle_event_id_pair = default_state_handle_event_id_pair>
    class automaton
    {
    public:
        using id_type = _Id;
        using event_type = _Event;
        using state_type = _State;
        using event_id_type = typename event_type::id_type;
        using state_id_type = typename state_type::id_type;
        using state_handle_type = typename state_type::handle_type;

        struct transition
        {
            automaton* target;
            state_id_type from;
            state_id_type to;
            event_id_type event;

            using vector = std::vector<transition>;

            constexpr transition() noexcept: target {}, from {}, to {}, event {} {}
            constexpr transition(const state_id_type from, const event_id_type event, const state_id_type to,
                                 automaton* const target = nullptr) noexcept:
                target(target),
                from(from),
                to(to),
                event(event)
            {
            }
        };

        // Target state of a transition (i.e. go to the 'state' which belongs in 'fsm').
        struct transition_target
        {
            state_handle_type state {};
            automaton* fsm {};
        };

        struct awaitable
        {
            automaton* self {};
            constexpr bool await_ready() const noexcept { return false; }

            state_handle_type make_transition(const state_handle_type& from_state, const event_id_type on_event_id,
                                              transition_target to) const
            {
                // The event is typically being sent to a state owned by this FSM (i.e. self).
                // However, it may also be going to a state owned by another FSM.
                // The destination FSM is in TransitionTarget struct together with the state handle.
                if (to.fsm == self)
                { // The target state lives in this FSM.
                    self->state_ = to.state;

                    if (self->logger_)
                        self->logger_(self->id_, self->id_, from_state.promise().id, on_event_id, to.state.promise().id);

                    self->is_active_.store(true, std::memory_order_relaxed);
                    return to.state;
                }

                // The target state lives in another FSM.
                // Note: self FSM will suspend and self->state remains in the state where it left off when to.fsm took over.
                to.fsm->state_ = to.state; // to.fsm will resume.
                // Move the event to the target FSM. The event of the target FSM should be invalid.
                assert(to.fsm->event_.is_valid() == false);
                to.fsm->event_ = std::move(self->event_);

                if (self->logger_)
                    self->logger_(self->id_, to.fsm->id_, from_state.promise().id, to.fsm->event_.id(), to.state.promise().id);

                // Self is suspended and to.fsm is resumed.
                self->is_active_.store(false, std::memory_order_relaxed);
                to.fsm->is_active_.store(true, std::memory_order_relaxed);
                return to.state;
            }

            std::coroutine_handle<> await_suspend(state_handle_type from_state) const
            {
                const event_type& on_event = self->latest_event();
                // If a state emits an invalid event all states will remain suspended.
                // Consequently, the FSM will stopped. It can be restarted by calling send_event().
                if (on_event.is_valid())
                {
                    const auto on_event_id = on_event.id();
                    // Find the destination for {from_state, on_event}-pair.
                    if (auto it = self->transitions_.find({from_state, on_event_id}); it != self->transitions_.end())
                    {
                        return make_transition(from_state, on_event_id, it->second);
                    }

                    auto error_message = self->create_error_message();
                    error_message << "' can't find transition from state '" << from_state.promise().id << "' on event '" << on_event_id
                                  << "'.\nPlease fix the transition table.";
                    throw std::runtime_error(error_message.str());
                }

                self->is_active_.store(false, std::memory_order_relaxed);
                return std::noop_coroutine();
            }

            event_type await_resume()
            {
                if (self->event_.is_valid())
                    return std::move(self->event_);

                auto error_message = self->create_error_message();
                error_message << "An empty event has been sent to state " << self->state_id();
                throw std::runtime_error(error_message.str());
            }
        };

        struct intial_awaitable
        {
            automaton* self {};
            constexpr bool await_ready() const noexcept { return false; }
            void await_suspend(state_handle_type) noexcept {}
            event_type await_resume()
            {
                self->is_active_.store(true, std::memory_order_relaxed);
                if (self->event_.is_valid())
                    return std::move(self->event_);

                auto error_message = self->create_error_message();
                error_message << "An empty event has been sent to state " << self->state_id();
                throw std::runtime_error(error_message.str());
            }
        };

        using logger_functor = std::function<void(const id_type fsm, const id_type target_fsm, const state_id_type from_state,
                                                  const event_id_type on_event_id, const state_id_type to_state)>;

        // It construct an FSM with an id.
        automaton(const id_type id = {}): id_(id) {}

        automaton(const automaton&) = delete;
        automaton(automaton&&) noexcept = default;
        automaton& operator= (const automaton&) = delete;
        automaton& operator= (automaton&&) noexcept = default;
        ~automaton() = default;

        id_type id() const noexcept { return id_; }

        // It returns true if the FSM is running and false if all states
        // are suspended and waiting for an event.
        bool is_active() const noexcept { return is_active_; }

        // The event that was sent in the latest transition.
        const event_type& latest_event() const noexcept { return event_; }

        // It returns the name of the target state of the latest transition.
        state_id_type state_id() const { return state_ ? state_.promise().id : state_id_type {}; }

        std::ostringstream create_error_message() const
        {
            std::ostringstream out {};
            out << "FSM('" << id_ << "'): ";
            return out;
        }

        // Sets the current state. The next event will come to this state.
        automaton& go_to(const state_type& state)
        {
            state_ = state.handle();
            return *this;
        }

        automaton& go_to(const state_id_type id)
        {
            state_ = find_handle(id);
            if (state_)
                return *this;

            auto error_message = create_error_message();
            error_message << std::source_location::current().function_name() << " did not find the requested state '" << id << '\'';
            throw std::runtime_error(error_message.str());
        }

        // It adds transition from state handle 'from' to state handle 'to' on event 'on_event' which lives in FSM 'target_fsm'.
        // target_fsm==nullptr means this FSM, so the 4th argument can be omitted if every state
        // refers to the same FSM.
        // It returns true if {from, on_event} pair has not been routed previously.
        // It returns false if an existing destination is replaced with '{to, target_fsm}'.
        // It should return typically true unless the state machine is deliberately modified on the fly.
        bool add_transition(const state_handle_type& from, const event_id_type on_event, const state_handle_type& to, automaton* target_fsm)
        {
            assert(target_fsm != nullptr);
            return transitions_.insert_or_assign({from, on_event}, transition_target {to, target_fsm}).second;
        }

        bool add_transition(const state_handle_type& from, const event_id_type on_event, const state_handle_type& to)
        {
            return add_transition(from, on_event, to, this);
        }

        // The same as above but the states are identified by state ids.
        bool add_transition(const state_id_type from_state, const event_id_type on_event, const state_id_type to_state,
                            automaton* target_fsm)
        {
            assert(target_fsm != nullptr);
            const state_handle_type from_handle = this->find_handle(from_state);
            if (!from_handle)
            {
                auto error_message = create_error_message();
                error_message << std::source_location::current().function_name() << " did not find the requested source state '"
                              << from_state << "'.";
                throw std::runtime_error(error_message.str());
            }

            const state_handle_type to_handle = target_fsm->find_handle(to_state);
            if (!to_handle)
            {
                auto error_message = create_error_message();
                error_message << std::source_location::current().function_name() << " did not find the requested target state '" << to_state
                              << "'.";
                throw std::runtime_error(error_message.str());
            }

            return add_transition(from_handle, on_event, to_handle, target_fsm);
        }

        bool add_transition(const state_id_type from_state, const event_id_type on_event, const state_id_type to_state)
        {
            return add_transition(from_state, on_event, to_state, this);
        }

        bool add_transition(const state_handle_type& from_handle, const event_id_type on_event, const state_id_type to_state,
                            automaton* const target_fsm)
        {
            assert(target_fsm != nullptr);
            const state_handle_type to_handle = target_fsm->find_handle(to_state);
            if (!to_handle)
            {
                auto error_message = create_error_message();
                error_message << std::source_location::current().function_name() << " did not find the requested target state'" << to_state
                              << '\'';
                throw std::runtime_error(error_message.str());
            }

            return add_transition(from_handle, on_event, to_handle, target_fsm);
        }

        bool add_transition(const state_handle_type& from_handle, const event_id_type on_event, const state_id_type to_state)
        {
            return add_transition(from_handle, on_event, to_state, this);
        }

        bool add_transition(const state_id_type from_state, const event_id_type on_event, const state_handle_type& to_handle,
                            automaton* const target_fsm)
        {
            assert(target_fsm != nullptr);
            const state_handle_type from_handle = this->find_handle(from_state);
            if (!from_handle)
            {
                auto error_message = create_error_message();
                error_message << std::source_location::current().function_name() << " did not find the requested source state'"
                              << from_state << '\'';
                throw std::runtime_error(error_message.str());
            }

            return add_transition(from_handle, on_event, to_handle, target_fsm);
        }

        bool add_transition(const state_id_type from_state, const event_id_type on_event, const state_handle_type& to_handle)
        {
            return add_transition(from_state, on_event, to_handle, this);
        }

        // A shortcut for writing "fsm << transition(from, event, to)" instead of "fsm.addTransition(from, event, to)".
        automaton& operator<< (const transition& transition)
        {
            const auto target = transition.target == nullptr ? this : transition.target;
            static_cast<void>(add_transition(transition.from, transition.event, transition.to, target));
            return *this;
        }

        // It removes transition triggered by event 'on_event' sent from 'from_state'.
        // It returns true if the transition was found and successfully removed.
        bool remove_transition(const state_handle_type& from_state, const event_id_type on_event)
        {
            const auto erased = transitions_.erase({from_state, on_event});
            return erased != 0U;
        }

        bool remove_transition(const state_id_type from_state, const event_id_type on_event)
        {
            const auto erased = transitions_.erase({find_handle(from_state), on_event});
            return erased != 0U;
        }

        // A shortcut for writing "fsm >> transition(from, event)" instead of "fsm.removeTransition(from, event)".
        automaton& operator>> (const transition& transition)
        {
            static_cast<void>(remove_transition(transition.from, transition.event));
            return *this;
        }

        // It returns true if the FSM knows how to deal with event 'on_event' sent from state 'from_state'.
        bool has_transition(const state_handle_type& from_state, const event_id_type on_event) const
        {
            return transitions_.contains({from_state, on_event});
        }

        bool has_transition(const state_id_type from_state, const event_id_type on_event) const
        {
            return transitions_.contains({find_handle(from_state), on_event});
        }

        // It returns a vector of transitions.
        transition::vector get_transitions() const
        {
            typename transition::vector result {};
            result.reserve(transitions_.size());
            for (const auto& [from_state_on_event, to_state]: transitions_)
                result.emplace_back(from_state_on_event.first.promise().id, from_state_on_event.second, to_state.state.promise().id,
                                    to_state.fsm);
            return result;
        }

        // It finds the target state of 'on_event' event id when it sent from state handle 'from_state'.
        // It returns an empty state id if the state is not found.
        std::optional<state_id_type> target_state(const state_handle_type& from_state, const event_id_type on_event) const noexcept
        {
            const auto it = transitions_.find({from_state, on_event});
            if (it != transitions_.end())
                return it->second.state.promise().id;
            return {};
        }

        // It finds the target state of 'on_event' event id when it sent from state id 'from_state'.
        // It returns an empty state id if the state is not found.
        std::optional<state_id_type> target_state(const state_id_type from_state, const event_id_type on_event) const
        {
            return target_state(find_handle(from_state), on_event);
        }

        // It emits the given event and returns an awaitable which gives
        // the next event sent to the awaiting state coroutine.
        awaitable emit_and_receive(event_type&& e)
        {
            event_ = std::move(e);
            return awaitable {this};
        }

        // It returns an awaitable which gives the next event sent to the awaiting state coroutine.
        intial_awaitable get_event() { return {this}; }

        // It adds a state to the state machine without associating any events with it.
        // It returns the index of the vector to which the state was stored.
        std::size_t add_state(state_type&& state)
        {
            if (has_state(state.id()))
            {
                std::ostringstream error_message {};
                error_message << "A state with id '" << state.id() << "' already exists in FSM " << id_;
                throw std::runtime_error(error_message.str());
            }

            if (state.handle())
            {
                states_.push_back(std::move(state));
                return states_.size() - 1U;
            }

            std::ostringstream error_message {};
            error_message << "Attempt to add an invalid state to FSM " << id_;
            throw std::runtime_error(error_message.str());
        }

        // Alias for the above.
        automaton& operator<< (state_type&& state)
        {
            add_state(std::move(state));
            return *this;
        }

        // It returns reference to the state object at the given index.
        const state_type& state_at(const std::size_t index) const { return states_.at(index); }

        // It returns the number of states in the FSM.
        std::size_t state_count() const noexcept { return states_.size(); }

        // It gets the states going from the initial suspension.
        automaton& start()
        {
            for (auto& state: states_)
                if (!state.is_started()) // Resume only if the coroutine is still suspended in initial_suspend.
                    state.handle().resume();
            return *this;
        }

        // It kicks off the state machine by sending the event.
        // It sends to the state which is either the state where the FSM left off when it was
        // suspended last time or the state which has been explicitly set by calling set_state().
        automaton& send_event(event_type&& event)
        {
            if (state_.promise().is_started)
            {
                event_ = std::move(event);
                state_.resume();
                return *this;
            }

            auto error_message = create_error_message();
            error_message << std::source_location::current().function_name() << '(' << event.id() << ") can not resume state "
                          << state_.promise().id << " because it has not been started. Call first fsm.start() to activate all states.";
            throw std::runtime_error(error_message.str());
        }

        // It finds the state based on state id.
        // It returns null if the id is not found.
        const state_type* find_state(const state_id_type state_id) const noexcept
        {
            // Find the name from the list of states
            for (const auto& state: states_)
                if (state.id() == state_id)
                    return &state;

            return nullptr;
        }

        static inline constexpr auto npos = std::size_t(~0U);

        // It finds the state vector index where the state with the given name lives.
        // It returns npos if the id is not found.
        std::size_t find_index(const state_id_type state_id) const noexcept
        {
            for (std::size_t i = 0U; i < states_.size(); ++i)
                if (states_[i].id() == state_id)
                    return i;

            return npos;
        }

        // It returns true if the given state is registered in the fsm.
        bool has_state(const state_id_type id) const noexcept
        {
            // Find the name from the list of states
            for (const state_type& state: states_)
                if (state.id() == id)
                    return true;
            return false;
        }

        // It gives access to the logger.
        const logger_functor& logger() const noexcept { return logger_; }
        // It sets the logger.
        void set_logger(logger_functor item) { logger_ = std::move(item); }

    private:
        using state_handle_event_id_pair = _State_handle_event_id_pair<state_handle_type, event_id_type>;
        using transition_map = std::unordered_map<state_handle_event_id_pair, transition_target, typename state_handle_event_id_pair::hash>;

        // Find the handle based on id. It returns an empty state handle if the id is not found.
        state_handle_type find_handle(const state_id_type id) const
        {
            // Find the name from the list of states
            for (const state_type& state: states_)
                if (state.id() == id)
                    return state.handle();
            return {};
        }

        logger_functor logger_ {};       // Callback for debugging and writing log. It is called when the state of
                                         // the fsm whose id is in the first argument is about to change from 'from_state' to 'to_state'
                                         // because the from_state is sending event 'on_event'.
        transition_map transitions_;     // Transition table in format {from-state, event} -> to-state. That is, an event sent from
                                         // from-state will be routed to to-state.
        std::vector<state_type> states_; // All coroutines which represent the states in the state machine.
        event_type event_;               // The latest event.
        state_handle_type state_ {};     // Current state (for information only).
        id_type id_;                     // Id of the FSM (for information only).
        std::atomic_bool is_active_ {};  // True if the FSM is running, false if suspended.
    };

    template <typename _FSM>
    typename _FSM::state_type coroutine(_FSM& fsm, auto event_handler)
    {
        for (auto event = co_await fsm.get_event();;) // Await for the first event.
        {
            if (!event.is_valid())
            {
                throw std::runtime_error("Unexpected invalid event");
            }

            event_handler(fsm, event);
            event = co_await fsm.emit_and_receive(std::move(event));
        }
    }
}
