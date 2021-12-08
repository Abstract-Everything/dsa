#include "array_widget.hpp"

namespace
{
constexpr float element_spacing = 1.0F;

[[nodiscard]] visual::Text_Widget invalid_text()
{
	const sf::Color     invalid_background{ 255, 50, 50 };
	visual::Text_Widget text;
	text.set_text("?");
	text.background(invalid_background);
	return text;
}

[[nodiscard]] visual::Text_Widget valid_text(std::string_view string)
{
	const sf::Color     valid_background{ 0, 150, 255 };
	visual::Text_Widget text;
	text.set_text(string);
	text.background(valid_background);
	return text;
}
} // namespace

namespace visual
{
Array_Widget::Array_Widget(
    std::uint64_t address,
    std::size_t   element_size,
    std::size_t   size)
    : Widget(element_spacing)
    , m_address(address)
    , m_element_size(element_size)
{
	resize(size);
}

std::uint64_t Array_Widget::address() const
{
	return m_address;
}

bool Array_Widget::contains(std::uint64_t address) const
{
	return index_of(address) < m_elements.size();
}

void Array_Widget::resize(std::size_t size)
{
	m_elements.resize(size, invalid_text());
}

sf::Vector2f Array_Widget::content_size() const
{
	sf::Vector2f size{ 0.0F, 0.0F };
	for (auto const &element : m_elements)
	{
		size.x += element.size().x;
		size.y = std::max(size.y, element.size().y);
	}
	return size;
}

void Array_Widget::content_draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	for (auto const &element : m_elements)
	{
		target.draw(element, states);
		states.transform.translate(
		    sf::Vector2f{ element.size().x + element_spacing, 0.0F });
	}
}

void Array_Widget::on_assignment(
    bool             initialised,
    std::uint64_t    address,
    std::string_view value)
{
	Text_Widget text;
	if (initialised)
	{
		text = valid_text(value);
	}
	else
	{
		text = invalid_text();
	}
	m_elements[index_of(address)] = text;
}

std::uint64_t Array_Widget::index_of(std::uint64_t address) const
{
	return (address - m_address) / m_element_size;
}

} // namespace visual
