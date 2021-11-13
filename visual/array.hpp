#ifndef VISUAL_ARRAY_HPP
#define VISUAL_ARRAY_HPP

#include <SFML/Graphics.hpp>

namespace visual
{
class Element : public sf::Drawable
{
 public:
	Element();

	void set_text(std::string_view string);

	sf::Vector2f size() const;

	void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

 private:
	sf::RectangleShape m_rectangle;
	sf::Text           m_text;

	void adjust();
};

class Array : public sf::Drawable
{
 public:
	explicit Array(std::size_t size);
	void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

 private:
	std::vector<Element> m_elements;
};

} // namespace visual

#endif
