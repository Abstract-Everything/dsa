#include "array_widget.hpp"

namespace
{
constexpr float element_spacing = 5.0F;
} // namespace

namespace visual
{
Array_Widget::Array_Widget(
    std::uint64_t address,
    std::size_t   element_size,
    std::size_t   size)
    : m_address(address)
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
	std::uint64_t index = (address - m_address) / m_element_size;
	return index < m_elements.size();
}

void Array_Widget::resize(std::size_t size)
{
	m_elements.resize(size);
}

void Array_Widget::update_element(
    bool             initialised,
    std::uint64_t    address,
    std::string_view value)
{
	std::uint64_t index = (address - m_address) / m_element_size;
	if (initialised)
	{
		m_elements[index].set_valid(value);
	}
	else
	{
		m_elements[index].set_invalid();
	}
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
