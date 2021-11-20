#include "viewport.hpp"

namespace visual
{
void Viewport::process(const Allocated_Array_Event &event)
{
	m_address      = event.address();
	m_element_size = event.element_size();
	m_visual_array.resize(event.size());
}

void Viewport::process(const Assignment_Event &event)
{
	std::uint64_t index = (event.address() - m_address) / m_element_size;
	m_visual_array.set_element(index, "0");
}

void Viewport::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	m_visual_array.draw(target, states);
}

} // namespace visual
