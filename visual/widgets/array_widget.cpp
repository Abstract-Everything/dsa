#include "array_widget.hpp"

namespace
{
constexpr float element_spacing = 5.0F;
} // namespace

namespace visual
{
Array_Widget::Array_Widget(std::size_t size)
{
	resize(size);
}

void Array_Widget::resize(std::size_t size)
{
	m_elements.resize(size);
}

void Array_Widget::set_element(std::size_t index, std::string_view value)
{
	m_elements[index].set_valid(value);
}

void Array_Widget::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	for (auto const &element : m_elements)
	{
		target.draw(element, states);
		states.transform.translate(
		    sf::Vector2f{ element.size().x + element_spacing, 0.0F });
	}
}

} // namespace visual
