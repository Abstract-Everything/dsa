#include "swap_event.hpp"

#include <fmt/format.h>

namespace visual
{

Swap_Event::Swap_Event(
    Address      lhs_address,
    Memory_Value lhs_value,
    Address      rhs_address,
    Memory_Value rhs_value)
    : m_lhs_address(lhs_address)
    , m_lhs_value(std::move(lhs_value))
    , m_rhs_address(rhs_address)
    , m_rhs_value(std::move(rhs_value))
{
}

Address Swap_Event::lhs_address() const
{
	return m_lhs_address;
}

const Memory_Value &Swap_Event::lhs_value() const
{
	return m_lhs_value;
}

Address Swap_Event::rhs_address() const
{
	return m_rhs_address;
}

const Memory_Value &Swap_Event::rhs_value() const
{
	return m_rhs_value;
}

std::string Swap_Event::to_string() const
{
	return fmt::format(
	    "Swap {0} at {1:#x} and {2} at {3:#x}",
	    m_lhs_value.value(),
	    m_lhs_address,
	    m_rhs_value.value(),
	    m_rhs_address);
}

} // namespace visual
