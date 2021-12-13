#ifndef VISUAL_MOVE_ASSIGNMENT_EVENT_HPP
#define VISUAL_MOVE_ASSIGNMENT_EVENT_HPP

#include "event.hpp"
#include "memory_value.hpp"

#include <cstdint>

namespace visual
{
class Move_Assignment_Event : public Event
{
 public:
	Move_Assignment_Event(
	    std::uint64_t to_address,
	    std::uint64_t from_address,
	    Memory_Value  value);

	~Move_Assignment_Event() override = default;

	[[nodiscard]] std::uint64_t       to_address() const;
	[[nodiscard]] std::uint64_t       from_address() const;
	[[nodiscard]] const Memory_Value &value() const;

 private:
	std::uint64_t m_to_address;
	std::uint64_t m_from_address;
	Memory_Value  m_value;
};

} // namespace visual

#endif
