#include "move_assignment_event.hpp"

#include <fmt/format.h>

namespace visual
{

Move_Assignment_Event::Move_Assignment_Event(
    Address      to_address,
    Address      from_address,
    Memory_Value value)
    : m_to_address(to_address)
    , m_from_address(from_address)
    , m_value(std::move(value))
{
}

Address Move_Assignment_Event::to_address() const
{
	return m_to_address;
}

Address Move_Assignment_Event::from_address() const
{
	return m_from_address;
}

const Memory_Value &Move_Assignment_Event::value() const
{
	return m_value;
}

std::string Move_Assignment_Event::to_string() const
{
	return fmt::format(
	    "Moved {0} from {1:#x} to {2:#x}",
	    m_value.value(),
	    m_from_address,
	    m_to_address);
}

} // namespace visual
