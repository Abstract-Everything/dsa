#include "deallocated_array_event.hpp"

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
} // namespace visual
