#ifndef VISUAL_MOVE_ASSIGNMENT_EVENT_HPP
#define VISUAL_MOVE_ASSIGNMENT_EVENT_HPP

#include "address.hpp"
#include "memory_value.hpp"

#include <string>

namespace visual
{
class Move_Assignment_Event
{
 public:
	Move_Assignment_Event(
	    Address      to_address,
	    Address      from_address,
	    Memory_Value value);

	[[nodiscard]] Address             to_address() const;
	[[nodiscard]] Address             from_address() const;
	[[nodiscard]] const Memory_Value &value() const;

	[[nodiscard]] std::string to_string() const;

 private:
	Address      m_to_address;
	Address      m_from_address;
	Memory_Value m_value;
};

} // namespace visual

#endif
