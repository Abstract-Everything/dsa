#ifndef VISUAL_COPY_ASSIGNMENT_EVENT_HPP
#define VISUAL_COPY_ASSIGNMENT_EVENT_HPP

#include "event.hpp"
#include "memory_value.hpp"

#include <cstdint>

namespace visual
{

class Copy_Assignment_Event : public Event
{
 public:
	Copy_Assignment_Event(std::uint64_t address, Memory_Value value);

	~Copy_Assignment_Event() override = default;

	[[nodiscard]] std::uint64_t       address() const;
	[[nodiscard]] const Memory_Value &value() const;

 private:
	std::uint64_t m_address;
	Memory_Value  m_value;
};

} // namespace visual

#endif
