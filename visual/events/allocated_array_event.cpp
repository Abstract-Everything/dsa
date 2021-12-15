#include "allocated_array_event.hpp"

#include <fmt/format.h>

namespace visual
{
Allocated_Array_Event::Allocated_Array_Event(
    Address     address,
    std::size_t element_size,
    std::size_t size)
    : m_address(address)
    , m_element_size(element_size)
    , m_size(size)
{
}

Address Allocated_Array_Event::address() const
{
	return m_address;
}

std::size_t Allocated_Array_Event::element_size() const
{
	return m_element_size;
}

std::size_t Allocated_Array_Event::size() const
{
	return m_size;
}

std::string Allocated_Array_Event::to_string() const
{
	return fmt::format(
	    "Allocated {0} elements of size {1} at {2:#x}",
	    m_size,
	    m_element_size,
	    m_address);
}

} // namespace visual
