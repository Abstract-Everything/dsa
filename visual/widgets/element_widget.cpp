#include "element_widget.hpp"

#include "main_window.hpp"

namespace
{
constexpr float element_padding = 5.0F;
} // namespace

namespace visual
{
Element_Widget::Element_Widget()
{
	m_text.setFillColor(sf::Color::White);
	m_text.setFont(Main_Window::instance().default_font());

	set_invalid();
}

void Element_Widget::set_invalid()
{
	set_text("?");
	m_rectangle.setFillColor(invalid_background);
}

void Element_Widget::set_valid(std::string_view string)
{
	set_text(string);
	m_rectangle.setFillColor(background);
}

void Element_Widget::set_text(std::string_view string)
{
	m_text.setString(std::string{ string });
	adjust();
}

sf::Vector2f Element_Widget::size() const
{
	return m_rectangle.getSize();
}

void Element_Widget::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	target.draw(m_rectangle, states);
	target.draw(m_text, states);
}

void Element_Widget::adjust()
{
	sf::Vector2f padding{ element_padding, element_padding };
	sf::Vector2f offset{ m_text.getLocalBounds().left,
			     m_text.getLocalBounds().top };
	m_text.setPosition(padding - offset);

	sf::Vector2f size{ m_text.getLocalBounds().width,
			   m_text.getLocalBounds().height };

	m_rectangle.setSize(padding + size + padding);
}

} // namespace visual
