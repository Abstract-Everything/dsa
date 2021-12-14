#include "move_assignment_event.hpp"

namespace visual
{

Move_Assignment_Event::Move_Assignment_Event(
    Address to_address,
    Address from_address,
    Memory_Value  value)
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

} // namespace visual
