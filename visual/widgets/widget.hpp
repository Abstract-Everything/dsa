#ifndef VISUAL_WIDGET_HPP
#define VISUAL_WIDGET_HPP

#include <SFML/Graphics.hpp>

#include <cstddef>
#include <memory>

namespace visual
{

class Widget : public sf::Drawable
{
 public:
	explicit Widget(
	    float            padding    = 0.0F,
	    const sf::Color &background = sf::Color::Transparent);

	~Widget() override = default;

	[[nodiscard]] virtual Widget *clone() const = 0;

	[[nodiscard]] sf::Vector2f size() const;
	void                       adjust();

	void background(const sf::Color &background);

	void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

 protected:
	[[nodiscard]] virtual sf::Vector2f content_size() const = 0;

	virtual void content_draw(
	    sf::RenderTarget &target,
	    sf::RenderStates  states) const = 0;

 private:
	float              m_padding;
	sf::RectangleShape m_rectangle;
};

} // namespace visual
#endif
