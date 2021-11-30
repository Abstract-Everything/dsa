#include "widget.hpp"

namespace visual
{

Widget::Widget(float padding, const sf::Color &background) : m_padding(padding)
{
	m_rectangle.setFillColor(background);
}

sf::Vector2f Widget::size() const
{
	sf::Vector2f padding{ m_padding, m_padding };
	return padding + content_size() + padding;
}

void Widget::adjust()
{
	m_rectangle.setSize(size());
}

void Widget::background(const sf::Color &background)
{
	m_rectangle.setFillColor(background);
}

void Widget::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	target.draw(m_rectangle, states);

	states.transform.translate(sf::Vector2f{ m_padding, m_padding });
	content_draw(target, states);
}

} // namespace visual
