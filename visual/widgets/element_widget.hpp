#ifndef VISUAL_ELEMENT_WIDGET_HPP
#define VISUAL_ELEMENT_WIDGET_HPP

#include "widget.hpp"

namespace visual
{

class Element_Widget : public Widget
{
 public:
	Element_Widget();

	void set_invalid();
	void set_valid(std::string_view string);

 protected:
	[[nodiscard]] sf::Vector2f content_size() const override;

	void content_draw(sf::RenderTarget &target, sf::RenderStates states)
	    const override;

 private:
	const sf::Color invalid_background{ 255, 50, 50 };
	const sf::Color valid_background{ 0, 150, 255 };

	sf::Text m_text;

	void set_text(std::string_view string);
};

} // namespace visual
#endif
