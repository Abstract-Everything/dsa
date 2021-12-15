#include "deallocated_array_event.hpp"

#include <fmt/format.h>

namespace visual
{

Deallocated_Array_Event::Deallocated_Array_Event(Address address)
    : m_address(address)
{
}

Address Deallocated_Array_Event::address() const
{
	return m_address;
}

std::string Deallocated_Array_Event::to_string() const
{
	return fmt::format("Deallocate {0:#x}", m_address);
}

} // namespace visual
