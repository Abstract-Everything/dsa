#ifndef VISUAL_DEALLOCATED_ARRAY_EVENT_HPP
#define VISUAL_DEALLOCATED_ARRAY_EVENT_HPP

#include "event.hpp"

#include <cstdint>

namespace visual
{
class Deallocated_Array_Event : public Event
{
 public:
	Deallocated_Array_Event(std::uint64_t address);

	~Deallocated_Array_Event() override = default;

	[[nodiscard]] std::uint64_t address() const;

 private:
	std::uint64_t m_address;
};

} // namespace visual

#endif
