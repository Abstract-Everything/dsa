#ifndef VISUAL_COPY_ASSIGNMENT_EVENT_HPP
#define VISUAL_COPY_ASSIGNMENT_EVENT_HPP

#include "address.hpp"
#include "memory_value.hpp"

#include <cstdint>

namespace visual
{

class Copy_Assignment_Event
{
 public:
	Copy_Assignment_Event(Address address, Memory_Value value);

	[[nodiscard]] Address             address() const;
	[[nodiscard]] const Memory_Value &value() const;

 private:
	Address      m_address;
	Memory_Value m_value;
};

} // namespace visual

#endif
