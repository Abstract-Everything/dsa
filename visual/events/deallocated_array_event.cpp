#include "deallocated_array_event.hpp"

namespace visual
{
Deallocated_Array_Event::Deallocated_Array_Event(std::uint64_t address)
    : m_address(address)
{
}

std::uint64_t Deallocated_Array_Event::address() const
{
	return m_address;
}
} // namespace visual
