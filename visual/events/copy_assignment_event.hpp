#ifndef VISUAL_COPY_ASSIGNMENT_EVENT_HPP
#define VISUAL_COPY_ASSIGNMENT_EVENT_HPP

#include "event.hpp"

#include <cstdint>
#include <string>
#include <string_view>

namespace visual
{
class Copy_Assignment_Event : public Event
{
 public:
	explicit Copy_Assignment_Event(
	    bool          initialised,
	    std::uint64_t address,
	    std::string   value);

	~Copy_Assignment_Event() override = default;

	[[nodiscard]] bool             initialised() const;
	[[nodiscard]] std::uint64_t    address() const;
	[[nodiscard]] std::string_view value() const;

 private:
	bool          m_initialised;
	std::uint64_t m_address;
	std::string   m_value;
};

} // namespace visual

#endif
