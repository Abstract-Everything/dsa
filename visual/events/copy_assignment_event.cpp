#include "copy_assignment_event.hpp"

namespace visual
{
Copy_Assignment_Event::Copy_Assignment_Event(
    bool          initialised,
    std::uint64_t address,
    std::string   value)
    : m_initialised(initialised)
    , m_address(address)
    , m_value(std::move(value))
{
}

bool Copy_Assignment_Event::initialised() const
{
	return m_initialised;
}

std::uint64_t Copy_Assignment_Event::address() const
{
	return m_address;
}

std::string_view Copy_Assignment_Event::value() const
{
	return m_value;
}

} // namespace visual
