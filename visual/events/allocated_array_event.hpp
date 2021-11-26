#ifndef VISUAL_ALLOCATED_ARRAY_EVENT_HPP
#define VISUAL_ALLOCATED_ARRAY_EVENT_HPP

#include "event.hpp"

#include <cstdint>

namespace visual
{
class Allocated_Array_Event : public Event
{
 public:
	Allocated_Array_Event(
	    std::uint64_t address,
	    std::size_t   element_size,
	    std::size_t   size);

	~Allocated_Array_Event() override = default;

	[[nodiscard]] std::uint64_t address() const;
	[[nodiscard]] std::size_t   element_size() const;
	[[nodiscard]] std::size_t   size() const;

 private:
	std::uint64_t m_address;
	std::size_t   m_element_size;
	std::size_t   m_size;
};

} // namespace visual

#endif
