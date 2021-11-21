#ifndef VISUAL_MOVE_ASSIGNMENT_EVENT_HPP
#define VISUAL_MOVE_ASSIGNMENT_EVENT_HPP

#include "event.hpp"

#include <cstdint>
#include <string>
#include <string_view>

namespace visual
{
class Move_Assignment_Event : public Event
{
 public:
	explicit Move_Assignment_Event(
	    bool          initialised,
	    std::uint64_t to_address,
	    std::uint64_t from_address,
	    std::string   value);

	~Move_Assignment_Event() override = default;

	[[nodiscard]] bool             initialised() const;
	[[nodiscard]] std::uint64_t    to_address() const;
	[[nodiscard]] std::uint64_t    from_address() const;
	[[nodiscard]] std::string_view value() const;

 private:
	bool          m_initialised;
	std::uint64_t m_to_address;
	std::uint64_t m_from_address;
	std::string   m_value;
};

} // namespace visual

#endif
