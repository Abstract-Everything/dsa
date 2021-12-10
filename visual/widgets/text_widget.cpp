#include "text_widget.hpp"

#include "main_window.hpp"

namespace
{
constexpr float element_padding = 5.0F;
} // namespace

namespace visual
{

Text_Widget::Text_Widget() : Widget(element_padding)
{
	m_text.setFillColor(sf::Color::White);
	m_text.setFont(Main_Window::instance().default_font());
}

Text_Widget* Text_Widget::clone() const
{
	return new Text_Widget(*this);
}

void Text_Widget::set_text(std::string_view string)
{
	m_text.setString(std::string{ string });
	adjust();
}

sf::Vector2f Text_Widget::content_size() const
{
	return { m_text.getLocalBounds().width, m_text.getLocalBounds().height };
}

void Text_Widget::content_draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	states.transform.translate(
	    -m_text.getLocalBounds().left,
	    -m_text.getLocalBounds().top);
	target.draw(m_text, states);
}

} // namespace visual
