#ifndef VISUAL_VIEWPORT_HPP
#define VISUAL_VIEWPORT_HPP

#include "allocated_array_event.hpp"
#include "array_widget.hpp"
#include "buffer.hpp"
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
	using Element_Widget = Widget;
	using Buffer_Widget  = Array_Widget<Widget>;
	using Memory_Widget  = Array_Widget<Buffer_Widget>;
	using Buffers        = std::vector<Buffer>;

 public:
	void add_event(std::unique_ptr<Event> &&event);
	void update(std::chrono::microseconds deltaTime);

	void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

 private:
	std::list<std::unique_ptr<Event>> m_events;
	Memory_Widget                     m_arrays{ Draw_Direction::Vertical };
	Buffers                           m_buffers;
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

	[[nodiscard]] bool update_element(
	    std::string_view log_message,
	    std::uint64_t    address,
	    bool             initialised,
	    std::string_view string);

	[[nodiscard]] bool update_element(
	    std::uint64_t    address,
	    bool             initialised,
	    std::string_view string);
};

} // namespace visual

#endif
