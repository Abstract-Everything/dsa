#include "copy_assignment_event.hpp"

#include <fmt/format.h>

namespace visual
{

Copy_Assignment_Event::Copy_Assignment_Event(Address address, Memory_Value value)
    : m_address(address)
    , m_value(std::move(value))
{
}

Address Copy_Assignment_Event::address() const
{
	return m_address;
}

const Memory_Value &Copy_Assignment_Event::value() const
{
	return m_value;
}

std::string Copy_Assignment_Event::to_string() const
{
	return fmt::format("Copy {0} to {1:#x}", m_value.value(), m_address);
}

} // namespace visual
