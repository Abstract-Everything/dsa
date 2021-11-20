#ifndef VISUAL_ASSIGNMENT_EVENT_HPP
#define VISUAL_ASSIGNMENT_EVENT_HPP

#include "event.hpp"

#include <cstdint>

namespace visual
{
class Assignment_Event : public Event
{
 public:
	explicit Assignment_Event(std::uint64_t address);

	~Assignment_Event() override = default;

	[[nodiscard]] std::uint64_t address() const;

 private:
	std::uint64_t m_address;
};

} // namespace visual

#endif
