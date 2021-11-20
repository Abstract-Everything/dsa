#ifndef VISUAL_ARRAY_WIDGET_HPP
#define VISUAL_ARRAY_WIDGET_HPP

#include "element_widget.hpp"

#include <SFML/Graphics.hpp>

#include <vector>

namespace visual
{
class Array_Widget : public sf::Drawable
{
 public:
	explicit Array_Widget(std::size_t size);

	void resize(std::size_t size);

	void set_element(std::size_t index, std::string_view value);

	void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

 private:
	std::vector<Element_Widget> m_elements{};
};

} // namespace visual

#endif
