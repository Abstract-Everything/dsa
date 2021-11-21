#include "viewport.hpp"

#include <spdlog/spdlog.h>

#include <exception>

namespace visual
{
void Viewport::process(const Allocated_Array_Event &event)
{
	for (auto const &array : m_arrays)
	{
		if (array.address() == event.address())
		{
			spdlog::warn("Received a duplicated allocate event on");
			return;
		}
	}

	m_arrays.emplace_back(event.address(), event.element_size(), event.size());
}

void Viewport::process(const Move_Assignment_Event &event)
{
	for (auto &array : m_arrays)
	{
		if (array.contains(event.address()))
		{
			array.update_element(
			    event.initialised(),
			    event.address(),
			    event.value());
			return;
		}
	}

	spdlog::warn("Received an assignment event for an address outside any "
		     "range");
}

void Viewport::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	for (auto const &array : m_arrays)
	{
		array.draw(target, states);
		states.transform.translate(sf::Vector2f{ 0.0F, 50.0F });
	}
}

} // namespace visual
