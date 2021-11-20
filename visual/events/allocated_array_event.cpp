#include "allocated_array_event.hpp"

namespace visual
{
Allocated_Array_Event::Allocated_Array_Event(
    std::uint64_t address,
    std::size_t   element_size,
    std::size_t   size)
    : m_address(address)
    , m_element_size(element_size)
    , m_size(size)
{
}

std::uint64_t Allocated_Array_Event::address() const
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

} // namespace visual
