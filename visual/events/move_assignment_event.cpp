#include "move_assignment_event.hpp"

namespace visual
{
Move_Assignment_Event::Move_Assignment_Event(
    bool          initialised,
    std::uint64_t to_address,
    std::uint64_t from_address,
    std::string   value)
    : m_initialised(initialised)
    , m_to_address(to_address)
    , m_from_address(from_address)
    , m_value(std::move(value))
{
}

bool Move_Assignment_Event::initialised() const
{
	return m_initialised;
}

std::uint64_t Move_Assignment_Event::to_address() const
{
	return m_to_address;
}

std::uint64_t Move_Assignment_Event::from_address() const
{
	return m_from_address;
}

std::string_view Move_Assignment_Event::value() const
{
	return m_value;
}

} // namespace visual
