#include "viewport.hpp"

#include <imgui.h>
#include <spdlog/spdlog.h>

#include <exception>

namespace
{

constexpr std::chrono::seconds event_duration{ 2 };

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

void Viewport::draw() const
{
	if (!ImGui::CollapsingHeader(
		"Structure Contents",
		ImGuiTreeNodeFlags_DefaultOpen))
	{
		return;
	}

	ImGui::Indent();

	int index = 0;
	for (auto const &buffer : m_buffers)
	{
		if (buffer.size() == 0)
		{
			// ImGui:: Does not like 0 size tables
			continue;
		}

		std::string table = fmt::format("Table{}", index++);
		if (!ImGui::BeginTable(
			table.c_str(),
			static_cast<int>(buffer.size()),
			ImGuiTableFlags_RowBg))
		{
			continue;
		}

		for (auto const &element : buffer)
		{
			ImGui::TableNextColumn();
			std::string_view value = element.value();

			const ImVec4 valid_background{ 0.0F, 0.65F, 1.0F, 1.0F };
			const ImVec4 invalid_background{ 1.0F, 0.2F, 0.2F, 1.0F };

			ImU32 background = ImGui::GetColorU32(
			    element.initialised() ? valid_background
						  : invalid_background);

			ImGui::TableSetBgColor(
			    ImGuiTableBgTarget_CellBg,
			    background);

			ImGui::TextUnformatted(
			    value.data(),
			    value.data() + value.length());
		}
		ImGui::EndTable();
	}

	ImGui::Unindent();
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

	return true;
}

bool Viewport::process(const Copy_Assignment_Event &event)
{
	return update_element(
	    "Received an assignment event for an address outside any range",
	    event.address(),
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
	    event.value());
}

bool Viewport::updated_moved_from_element(const Move_Assignment_Event &event)
{
	return update_element(event.from_address(), Memory_Value());
}

bool Viewport::update_element(
    std::string_view    log_message,
    std::uint64_t       address,
    const Memory_Value &value)
{
	bool success = update_element(address, value);
	if (!success)
	{
		spdlog::warn(log_message);
	}
	return success;
}

bool Viewport::update_element(std::uint64_t address, const Memory_Value &value)
{
	auto buffer_it = std::find_if(
	    m_buffers.begin(),
	    m_buffers.end(),
	    [address](const Buffer &buffer) { return buffer.contains(address); });

	if (buffer_it >= m_buffers.end())
	{
		return false;
	}

	auto element_it =
	    buffer_it->begin()
	    + static_cast<std::ptrdiff_t>(buffer_it->index_of(address));

	if (element_it >= buffer_it->end())
	{
		spdlog::error("Tried to update an element outside of a buffer");
		return false;
	}

	*element_it = value;

	return true;
}

} // namespace visual
