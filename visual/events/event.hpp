#ifndef VISUAL_EVENT_HPP
#define VISUAL_EVENT_HPP

#include <cstdint>
#include <memory>

namespace visual
{
class Event
{
 public:
	static void Dispatch(std::unique_ptr<Event> event);

	virtual ~Event() = default;

	[[nodiscard]] const char *name() const;
};

} // namespace visual

#endif
