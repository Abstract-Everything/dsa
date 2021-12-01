#include "element_widget.hpp"

#include "main_window.hpp"

namespace
{
constexpr float element_padding = 5.0F;
} // namespace

namespace visual
{

Element_Widget::Element_Widget() : Widget(element_padding)
{
	m_text.setFillColor(sf::Color::White);
	m_text.setFont(Main_Window::instance().default_font());

	set_invalid();
}

void Element_Widget::set_invalid()
{
	set_text("?");
	background(invalid_background);
}

void Element_Widget::set_valid(std::string_view string)
{
	set_text(string);
	background(valid_background);
}

sf::Vector2f Element_Widget::content_size() const
{
	return { m_text.getLocalBounds().width, m_text.getLocalBounds().height };
}

void Element_Widget::content_draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	states.transform.translate(
	    -m_text.getLocalBounds().left,
	    -m_text.getLocalBounds().top);
	target.draw(m_text, states);
}

void Element_Widget::set_text(std::string_view string)
{
	m_text.setString(std::string{ string });
	adjust();
}

} // namespace visual
