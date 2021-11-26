#ifndef VISUAL_ELEMENT_WIDGET_HPP
#define VISUAL_ELEMENT_WIDGET_HPP

#include <SFML/Graphics.hpp>

namespace visual
{
class Element_Widget : public sf::Drawable
{
 public:
	Element_Widget();

	void set_invalid();
	void set_valid(std::string_view string);
	void set_text(std::string_view string);

	sf::Vector2f size() const;

	void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

 private:
	const sf::Color invalid_background{ 255, 50, 50 };
	const sf::Color background{ 0, 150, 255 };

	sf::RectangleShape m_rectangle;
	sf::Text           m_text;

	void adjust();
};

} // namespace visual
#endif
