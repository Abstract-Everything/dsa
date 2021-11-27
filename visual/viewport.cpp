#include "viewport.hpp"

#include <spdlog/spdlog.h>

#include <exception>

namespace visual
{
void Viewport::add_event(std::unique_ptr<Event> &&event)
{
	m_events.push_back(std::move(event));
}

void Viewport::process_events()
{
	for (auto const &event : m_events)
	{
		if (auto const *allocated_array =
			dynamic_cast<Allocated_Array_Event const *>(event.get()))
		{
			process(*allocated_array);
		}
		else if (
		    auto const *move_assignment =
			dynamic_cast<Move_Assignment_Event const *>(event.get()))
		{
			process(*move_assignment);
		}
		else if (
		    auto const *copy_assignment =
			dynamic_cast<Copy_Assignment_Event const *>(event.get()))
		{
			process(*copy_assignment);
		}
		else if (
		    auto const *deallocated_array =
			dynamic_cast<Deallocated_Array_Event const *>(event.get()))
		{
			process(*deallocated_array);
		}
		else
		{
			Event const *event_ptr = event.get();
			spdlog::error(
			    "Unhandled event of type {}",
			    typeid(event_ptr).name());
		}
	}
	m_events.clear();
}

void Viewport::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	for (auto const &array : m_arrays)
	{
		array.draw(target, states);
		states.transform.translate(sf::Vector2f{ 0.0F, 50.0F });
	}
}

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

void Viewport::process(const Deallocated_Array_Event &event)
{
	auto it = std::find_if(
	    m_arrays.begin(),
	    m_arrays.end(),
	    [&event](const Array_Widget &array)
	    { return array.address() == event.address(); });

	if (it == m_arrays.end())
	{
		spdlog::warn(
		    "Received a deallocate event on a non monitored address");
		return;
	}

	m_arrays.erase(it);
}

void Viewport::process(const Copy_Assignment_Event &event)
{
	for (auto &array : m_arrays)
	{
		if (array.contains(event.address()))
		{
			array.on_assignment(
			    event.initialised(),
			    event.address(),
			    event.value());
			return;
		}
	}

	spdlog::warn(
	    "Received an assignment event for an address outside any range");
}

void Viewport::process(const Move_Assignment_Event &event)
{
	updated_moved_to_element(event);
	updated_moved_from_element(event);
}

void Viewport::updated_moved_to_element(const Move_Assignment_Event &event)
{
	for (auto &array : m_arrays)
	{
		if (array.contains(event.to_address()))
		{
			array.on_assignment(
			    event.initialised(),
			    event.to_address(),
			    event.value());
			return;
		}
	}

	spdlog::warn(
	    "Received a move assignment event for an address outside any "
	    "range");
}

void Viewport::updated_moved_from_element(const Move_Assignment_Event &event)
{
	for (auto &array : m_arrays)
	{
		if (array.contains(event.from_address()))
		{
			array.invalidate_element(event.from_address());
			return;
		}
	}
}

} // namespace visual
