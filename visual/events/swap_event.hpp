#ifndef SWAP_EVENT_HPP
#define SWAP_EVENT_HPP

#include "address.hpp"
#include "memory_value.hpp"

#include <string>

namespace visual
{

class Swap_Event
{
 public:
	Swap_Event(
	    Address      lhs_address,
	    Memory_Value lhs_value,
	    Address      rhs_address,
	    Memory_Value rhs_value);

	[[nodiscard]] Address             lhs_address() const;
	[[nodiscard]] const Memory_Value &lhs_value() const;
	[[nodiscard]] Address             rhs_address() const;
	[[nodiscard]] const Memory_Value &rhs_value() const;

	[[nodiscard]] std::string to_string() const;

 private:
	Address      m_lhs_address;
	Memory_Value m_lhs_value;
	Address      m_rhs_address;
	Memory_Value m_rhs_value;
};

} // namespace visual

#endif
