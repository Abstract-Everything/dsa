#ifndef VISUAL_VIEWPORT_HPP
#define VISUAL_VIEWPORT_HPP

#include "allocated_array_event.hpp"
#include "array_widget.hpp"
#include "move_assignment_event.hpp"

#include <SFML/Graphics/Drawable.hpp>

namespace visual
{
class Viewport : public sf::Drawable
{
 public:
	void process(const Allocated_Array_Event &event);
	void process(const Move_Assignment_Event &event);

	void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

 private:
	std::vector<Array_Widget> m_arrays;

	void updated_moved_to_element(const Move_Assignment_Event &event);
	void updated_moved_from_element(const Move_Assignment_Event &event);
};

} // namespace visual

#endif
