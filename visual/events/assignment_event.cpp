#include "assignment_event.hpp"

namespace visual
{
Assignment_Event::Assignment_Event(std::uint64_t address) : m_address(address)
{
}

std::uint64_t Assignment_Event::address() const
{
	return m_address;
}

} // namespace visual
