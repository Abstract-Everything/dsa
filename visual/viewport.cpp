#include "viewport.hpp"

#include <spdlog/spdlog.h>

#include <exception>

namespace
{
constexpr std::chrono::seconds event_duration{ 2 };

[[nodiscard]] visual::Text_Widget invalid_element()
{
	const sf::Color     invalid_background{ 255, 50, 50 };
	visual::Text_Widget text;
	text.set_text("?");
	text.background(invalid_background);
	return text;
}

[[nodiscard]] visual::Text_Widget valid_element(std::string_view string)
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
	m_arrays.draw(target, states);
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
	Buffer buffer{ event.address(), event.size(), event.element_size() };

	if (std::find_if(
		m_buffers.begin(),
		m_buffers.end(),
		[&buffer](const Buffer &other)
		{ return Buffer::overlap(other, buffer); })
	    != m_buffers.end())
	{
		spdlog::warn("Received a duplicated allocate event");
		return false;
	}

	m_buffers.push_back(buffer);
	m_arrays.push_back(std::make_unique<Array_Widget<Widget>>(
	    buffer.count(),
	    invalid_element(),
	    Draw_Direction::Horizontal));

	return true;
}

bool Viewport::process(const Deallocated_Array_Event &event)
{
	auto buffer_it = std::find_if(
	    m_buffers.begin(),
	    m_buffers.end(),
	    [&event](const Buffer &buffer)
	    { return buffer.contains(event.address()); });

	if (buffer_it == m_buffers.end())
	{
		spdlog::warn(
		    "Received a deallocate event on a non monitored address");
		return false;
	}

	m_buffers.erase(buffer_it);
	m_arrays.erase(
	    m_arrays.begin() + std::distance(m_buffers.begin(), buffer_it));

	return true;
}

bool Viewport::process(const Copy_Assignment_Event &event)
{
	return update_element(
	    "Received an assignment event for an address outside any range",
	    event.address(),
	    event.initialised(),
	    event.value());
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
	return update_element(
	    "Received a move assignment event for an address outside any range",
	    event.to_address(),
	    event.initialised(),
	    event.value());
}

bool Viewport::updated_moved_from_element(const Move_Assignment_Event &event)
{
	return update_element(event.from_address(), false, "");
}

bool Viewport::update_element(
    std::string_view log_message,
    std::uint64_t    address,
    bool             initialised,
    std::string_view string)
{
	bool success = update_element(address, initialised, string);
	if (!success)
	{
		spdlog::warn(log_message);
	}
	return success;
}

bool Viewport::update_element(
    std::uint64_t    address,
    bool             initialised,
    std::string_view string)
{
	auto buffer_it = std::find_if(
	    m_buffers.begin(),
	    m_buffers.end(),
	    [address](const Buffer &buffer) { return buffer.contains(address); });

	if (buffer_it >= m_buffers.end())
	{
		return false;
	}

	auto arrays_it =
	    m_arrays.begin() + std::distance(m_buffers.begin(), buffer_it);

	if (arrays_it >= m_arrays.end())
	{
		spdlog::error(
		    "A buffer does not have a corresponding array "
		    "representation.");
		return false;
	}

	std::unique_ptr<Buffer_Widget> &buffer_widget = *arrays_it;

	auto element_it =
	    buffer_widget->begin()
	    + static_cast<std::ptrdiff_t>(buffer_it->index_of(address));

	if (element_it >= buffer_widget->end())
	{
		spdlog::error(
		    "A buffer element does not have a corresponding "
		    "representation");
		return false;
	}

	*element_it = std::make_unique<Text_Widget>(
	    initialised ? valid_element(string) : invalid_element());
	return true;
}

} // namespace visual
