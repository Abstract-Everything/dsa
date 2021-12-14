#ifndef VISUAL_VIEWPORT_HPP
#define VISUAL_VIEWPORT_HPP

#include "allocated_array_event.hpp"
#include "buffer.hpp"
#include "copy_assignment_event.hpp"
#include "deallocated_array_event.hpp"
#include "event.hpp"
#include "move_assignment_event.hpp"

#include <SFML/Graphics/Drawable.hpp>

#include <chrono>
#include <list>

namespace visual
{

class Viewport
{
 public:
	void add_event(Event &&event);
	void update(std::chrono::microseconds deltaTime);

	void draw() const;

 private:
	std::list<Event>          m_events;
	std::vector<Buffer>       m_buffers;
	std::chrono::microseconds m_eventTimeout{ -1 };

	[[nodiscard]] bool process(const Event &event);
	[[nodiscard]] bool process(const Allocated_Array_Event &event);
	[[nodiscard]] bool process(const Deallocated_Array_Event &event);
	[[nodiscard]] bool process(const Copy_Assignment_Event &event);
	[[nodiscard]] bool process(const Move_Assignment_Event &event);

	[[nodiscard]] bool updated_moved_to_element(
	    const Move_Assignment_Event &event);

	[[nodiscard]] bool updated_moved_from_element(
	    const Move_Assignment_Event &event);

	[[nodiscard]] bool update_element(
	    std::string_view    log_message,
	    std::uint64_t       address,
	    const Memory_Value &value);

	[[nodiscard]] bool update_element(
	    std::uint64_t       address,
	    const Memory_Value &value);
};

} // namespace visual

#endif
