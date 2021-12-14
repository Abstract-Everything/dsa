#include "copy_assignment_event.hpp"

namespace visual
{

Copy_Assignment_Event::Copy_Assignment_Event(std::uint64_t address, Memory_Value value)
    : m_address(address)
    , m_value(std::move(value))
{
}

std::uint64_t Copy_Assignment_Event::address() const
{
	return m_address;
}

const Memory_Value &Copy_Assignment_Event::value() const
{
	return m_value;
}

} // namespace visual
