#include "viewport.hpp"

#include <imgui.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <exception>

namespace
{

constexpr std::chrono::seconds event_duration{ 2 };
constexpr ImU32                valid_background   = IM_COL32(0, 150, 255, 255);
constexpr ImU32                invalid_background = IM_COL32(255, 50, 50, 255);

ImU32 cell_background(bool initialised)
{
	return initialised ? valid_background : invalid_background;
}

void draw_buffer_size(const visual::Memory_Allocation &buffer)
{
	ImGui::TableNextColumn();
	std::string size = std::to_string(buffer.size());
	ImGui::TextUnformatted(size.c_str());
}

// We use a table to represent multiple values in a buffer element. For example
// a buffer having elements with mutliple values can be represented in different
// rows as:
//
// Element 1: Value A, Element 2: Value A, Element 3: Value A,
//            Value B,            Value B,            Value B, ....
//            Value C,            Value C,            Value C,
//
// Using a single table was the best way found to do it because ImGui does not
// auto scale the cells in nested tables.
void draw_buffer_elements(const visual::Memory_Allocation &buffer)
{
	for (std::size_t i = 0; i < buffer.max_element_size(); ++i)
	{
		if (i != 0)
		{
			ImGui::TableNextRow();
		}

		ImGui::TableSetColumnIndex(0);

		for (auto const &element : buffer)
		{
			ImGui::TableNextColumn();

			if (element.size() <= i)
			{
				continue;
			}

			auto const      &value     = element[i];
			std::string_view value_str = value.value();

			ImGui::TableSetBgColor(
			    ImGuiTableBgTarget_CellBg,
			    cell_background(value.initialised()));

			ImGui::TextUnformatted(
			    value_str.data(),
			    value_str.data() + value_str.length());
		}
	}
}

void draw_buffer(const visual::Memory_Allocation &buffer)
{
	bool                   open         = true;
	const ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration
					      | ImGuiWindowFlags_AlwaysAutoResize
					      | ImGuiWindowFlags_NoNav;

	const ImGuiTableFlags table_flags = ImGuiTableFlags_SizingFixedFit
					    | ImGuiTableFlags_BordersInnerV
					    | ImGuiTableFlags_RowBg;

	const std::string window_name =
	    fmt::format("Buffer_{}", buffer.address());

	ImGui::Begin(window_name.c_str(), &open, window_flags);
	ImGui::SetWindowFontScale(2.0F);

	ImGui::BeginTable(
	    "Elements",
	    1 + static_cast<int>(buffer.size()),
	    table_flags);

	draw_buffer_size(buffer);
	draw_buffer_elements(buffer);

	ImGui::EndTable();

	ImGui::End();
}

} // namespace

namespace visual
{

void Viewport::add_event(Event &&event)
{
	spdlog::trace("Added eventof type: {}", to_string(event));
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
		if (process(event))
		{
			break;
		}
	}
}

void Viewport::draw() const
{
	for (auto const &buffer : m_memory)
	{
		draw_buffer(buffer);
	}
}

bool Viewport::process(const Event &event)
{
	spdlog::trace("Processing event of type: {}", to_string(event));

	return std::visit(
	    [this](auto &&event_typed) { return process(event_typed); },
	    event);
}

bool Viewport::process(const Allocated_Array_Event &event)
{
	m_memory.insert(Memory_Allocation{ event.address(),
					   event.size(),
					   event.element_size() });
	return true;
}

bool Viewport::process(const Deallocated_Array_Event &event)
{
	m_memory.erase(event.address());
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
	return update_element(
	    event.from_address(),
	    event.value().is_pointer()
		? Memory_Value(event.value().size(), false, 0U)
		: Memory_Value(event.value().size(), false, ""));
}

bool Viewport::update_element(
    std::string_view    log_message,
    Address             address,
    const Memory_Value &value)
{
	bool success = update_element(address, value);
	if (!success)
	{
		spdlog::warn(log_message);
	}
	return success;
}

bool Viewport::update_element(Address address, const Memory_Value &value)
{
	return m_memory.update_element(address, value);
}

} // namespace visual
