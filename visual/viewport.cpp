#include "viewport.hpp"

#include <spdlog/spdlog.h>

#include <exception>

namespace
{
constexpr std::chrono::seconds event_duration{ 2 };
}

namespace visual
{
void Viewport::add_event(std::unique_ptr<Event> &&event)
{
	spdlog::trace("Added eventof type: {}", event->name());
	m_events.push_back(std::move(event));
}

void Viewport::update(std::chrono::microseconds deltaTime)
{
	if (m_eventTimeout.count() > 0)
	{
		m_eventTimeout -= deltaTime;
		return;
	}

	m_eventTimeout = event_duration;

	while (!m_events.empty())
	{
		auto event = std::move(m_events.front());
		m_events.pop_front();
		if (process(*event))
		{
			break;
		}
	}
}

void Viewport::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	for (auto const &array : m_arrays)
	{
		array.draw(target, states);
		states.transform.translate(sf::Vector2f{ 0.0F, array.size().y });
	}
}

bool Viewport::process(const Event &event)
{
	spdlog::trace("Processing event of type: {}", event.name());

	if (auto const *allocated_array =
		dynamic_cast<Allocated_Array_Event const *>(&event))
	{
		return process(*allocated_array);
	}

	if (auto const *move_assignment =
		dynamic_cast<Move_Assignment_Event const *>(&event))
	{
		return process(*move_assignment);
	}

	if (auto const *copy_assignment =
		dynamic_cast<Copy_Assignment_Event const *>(&event))
	{
		return process(*copy_assignment);
	}

	if (auto const *deallocated_array =
		dynamic_cast<Deallocated_Array_Event const *>(&event))
	{
		return process(*deallocated_array);
	}

	spdlog::warn("Unhandled event of type {}", event.name());
	return false;
}

bool Viewport::process(const Allocated_Array_Event &event)
{
	for (auto const &array : m_arrays)
	{
		if (array.address() == event.address())
		{
			spdlog::warn("Received a duplicated allocate event");
			return false;
		}
	}

	m_arrays.emplace_back(event.address(), event.element_size(), event.size());
	return true;
}

bool Viewport::process(const Deallocated_Array_Event &event)
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
		return false;
	}

	m_arrays.erase(it);
	return true;
}

bool Viewport::process(const Copy_Assignment_Event &event)
{
	for (auto &array : m_arrays)
	{
		if (array.contains(event.address()))
		{
			array.on_assignment(
			    event.initialised(),
			    event.address(),
			    event.value());
			return true;
		}
	}

	spdlog::warn(
	    "Received an assignment event for an address outside any range");
	return false;
}

bool Viewport::process(const Move_Assignment_Event &event)
{
	// This prevents short circuting
	const bool moved_to   = updated_moved_to_element(event);
	const bool moved_from = updated_moved_from_element(event);
	return moved_to || moved_from;
}

bool Viewport::updated_moved_to_element(const Move_Assignment_Event &event)
{
	for (auto &array : m_arrays)
	{
		if (array.contains(event.to_address()))
		{
			array.on_assignment(
			    event.initialised(),
			    event.to_address(),
			    event.value());
			return true;
		}
	}

	spdlog::warn(
	    "Received a move assignment event for an address outside any "
	    "range");
	return false;
}

bool Viewport::updated_moved_from_element(const Move_Assignment_Event &event)
{
	for (auto &array : m_arrays)
	{
		if (array.contains(event.from_address()))
		{
			array.invalidate_element(event.from_address());
			return true;
		}
	}
	return false;
}

} // namespace visual
