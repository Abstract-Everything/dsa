#ifndef VISUAL_VIEWPORT_HPP
#define VISUAL_VIEWPORT_HPP

#include "allocated_array_event.hpp"
#include "array_widget.hpp"
#include "assignment_event.hpp"

#include <SFML/Graphics/Drawable.hpp>

namespace visual
{
class Viewport : public sf::Drawable
{
 public:
	void process(const Allocated_Array_Event &event);
	void process(const Assignment_Event &event);

	void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

 private:
	std::uint64_t m_address{ 0 };
	std::uint64_t m_element_size{ 0 };

	Array_Widget m_visual_array{ 0 };
};

} // namespace visual

#endif
