#include "assignment_event.hpp"

namespace visual
{
Assignment_Event::Assignment_Event(std::uint64_t address, std::string value)
    : m_address(address)
    , m_value(std::move(value))
{
}

std::uint64_t Assignment_Event::address() const
{
	return m_address;
}

std::string_view Assignment_Event::value() const
{
	return m_value;
}

} // namespace visual
