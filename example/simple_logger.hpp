#pragma once
#include <ostream>

namespace co_fsm
{
    // Helper for state transition tracing
    template <typename _Fsm>
    struct simple_logger
    {
        using event_id_type = _Fsm::event_id_type;
        using id_type = _Fsm::id_type;
        using state_id_type = _Fsm::state_id_type;

        std::ostream& stream;
        void operator() (const id_type fsm_id, const id_type target_fsm_id, const state_id_type from_state, const event_id_type on_event_id,
                         const state_id_type to_state)
        {
            stream << " [" << fsm_id;
            if (target_fsm_id != fsm_id)
            {
                stream << "-->" << target_fsm_id;
            }

            stream << "] event '" << on_event_id << "' sent from state '" << from_state << "' --> state '" << to_state << "'\n";
        }
    };
}
