#ifndef VISUAL_VIEWPORT_HPP
#define VISUAL_VIEWPORT_HPP

#include "allocated_array_event.hpp"
#include "array_widget.hpp"
#include "copy_assignment_event.hpp"
#include "deallocated_array_event.hpp"
#include "move_assignment_event.hpp"

#include <SFML/Graphics/Drawable.hpp>

#include <chrono>
#include <list>

namespace visual
{
class Viewport : public sf::Drawable
{
 public:
	void add_event(std::unique_ptr<Event> &&event);
	void update(std::chrono::microseconds deltaTime);

	void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

 private:
	std::list<std::unique_ptr<Event>> m_events;
	std::vector<Array_Widget>         m_arrays;
	std::chrono::microseconds         m_eventTimeout{ -1 };

	[[nodiscard]] bool process(const Event &event);
	[[nodiscard]] bool process(const Allocated_Array_Event &event);
	[[nodiscard]] bool process(const Deallocated_Array_Event &event);
	[[nodiscard]] bool process(const Copy_Assignment_Event &event);
	[[nodiscard]] bool process(const Move_Assignment_Event &event);

	[[nodiscard]] bool updated_moved_to_element(
	    const Move_Assignment_Event &event);

	[[nodiscard]] bool updated_moved_from_element(
	    const Move_Assignment_Event &event);
};

} // namespace visual

#endif
