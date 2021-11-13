#include "array.hpp"

#include "main_window.hpp"

#include <fmt/format.h>

namespace
{
constexpr float element_padding = 5.0F;
constexpr float element_spacing = 5.0F;
} // namespace

namespace visual
{
Element::Element()
{
	const sf::Color element_background{ 0, 150, 255 };
	m_rectangle.setFillColor(element_background);

	m_text.setFillColor(sf::Color::White);
	m_text.setFont(Main_Window::instance().default_font());
}

void Element::set_text(std::string_view string)
{
	m_text.setString(std::string{ string });
	adjust();
}

sf::Vector2f Element::size() const
{
	return m_rectangle.getSize();
}

void Element::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	target.draw(m_rectangle, states);
	target.draw(m_text, states);
}

void Element::adjust()
{
	sf::Vector2f padding{ element_padding, element_padding };
	sf::Vector2f offset{ m_text.getLocalBounds().left,
			     m_text.getLocalBounds().top };
	m_text.setPosition(padding - offset);

	sf::Vector2f size{ m_text.getLocalBounds().width,
			   m_text.getLocalBounds().height };

	m_rectangle.setSize(padding + size + padding);
}

Array::Array(std::size_t size)
{
	m_elements.resize(size);
	for (std::size_t i = 0; i < size; ++i)
	{
		auto &Element = m_elements[i];
		Element.set_text(fmt::format("{}", i));
	}
}

void Array::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	for (auto const &Element : m_elements)
	{
		target.draw(Element, states);
		states.transform.translate(
		    sf::Vector2f{ Element.size().x + element_spacing, 0.0F });
	}
}

} // namespace visual
