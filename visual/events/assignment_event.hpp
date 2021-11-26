#ifndef VISUAL_ASSIGNMENT_EVENT_HPP
#define VISUAL_ASSIGNMENT_EVENT_HPP

#include "event.hpp"

#include <cstdint>

namespace visual
{
class Assignment_Event : public Event
{
 public:
	explicit Assignment_Event(std::uint64_t address, std::string value);

	~Assignment_Event() override = default;

	[[nodiscard]] std::uint64_t    address() const;
	[[nodiscard]] std::string_view value() const;

 private:
	std::uint64_t m_address;
	std::string   m_value;
};

} // namespace visual

#endif
