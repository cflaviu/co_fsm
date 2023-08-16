#pragma once
#ifndef PCH
    #include <optional>
#endif

namespace co_fsm
{
    // Base class for event. The main feature of this class is event identification.
    // An event object is considered valid as long as his identifier is valid.
    // Since FSM is a template, it will require a single customized derived class from this class.
    // That means it is not possible to use multiple derived event classes inside a specific FSM.
    // Data polimorphism of a derived event class can be achieved, for example, using std::variant.
    template <typename _Id>
    class event_base
    {
    public:
        using id_type = _Id;
        using optional_id_type = std::optional<id_type>;

        event_base() = default;
        event_base(const event_base&) = default;
        event_base(event_base&& item) noexcept: id_(std::move(item.id_)) { item.invalidate(); }

        // It returns true if the event is empty (i.e. name string is not set)
        bool is_valid() const noexcept { return id_.has_value(); }

        // It returns the event id.
        id_type id() const noexcept { return id_.value(); }

        // It returns true if the name of the event == other
        bool has_same_id(const optional_id_type id) const noexcept { return id_ == id; }

        // It invalidates the event (i.e. it invalidates the identifier).
        void invalidate() noexcept { id_.reset(); }

        event_base& operator= (const event_base& item) noexcept = default;

        event_base& operator= (event_base&& item) noexcept
        {
            if (&item != this)
            {
                id_ = std::move(item.id_);
                item.invalidate(); // Invalidation of source event.
            }

            return *this;
        }

    protected:
        void set_id(const id_type id) noexcept { id_ = id; }

    private:
        // The event id. By default it is not set meaning the event is invalid.
        optional_id_type id_ {};
    };

    // It returns true if the events have same id. Otherwise it returns false.
    template <typename _Event>
    bool operator== (const _Event& event, const typename _Event::id_type id) noexcept
    {
        return event.has_same_id(id);
    }

    // It returns true if the events have same id. Otherwise it returns false.
    template <typename _Event>
    bool operator== (const typename _Event::id_type id, const _Event& event) noexcept
    {
        return event.has_same_id(id);
    }
}
