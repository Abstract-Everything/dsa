#ifndef VISUAL_EVENT_HPP
#define VISUAL_EVENT_HPP

#include "allocated_array_event.hpp"
#include "copy_assignment_event.hpp"
#include "deallocated_array_event.hpp"
#include "move_assignment_event.hpp"
#include "swap_event.hpp"

#include <variant>

namespace visual
{

using Event = std::variant<
    Allocated_Array_Event,
    Deallocated_Array_Event,
    Move_Assignment_Event,
    Copy_Assignment_Event,
    Swap_Event>;

void Dispatch(Event &&event);

std::string to_string(const Event &event);

} // namespace visual

#endif
