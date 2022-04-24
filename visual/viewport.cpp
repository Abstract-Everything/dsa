#include "viewport.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <exception>
#include <map>

namespace
{

class Table_Value
{
 public:
	ImGuiTable *table;
	int         column;
	int         row;
};

constexpr std::chrono::seconds event_duration{1};

constexpr float line_thickness = 5.0F;
constexpr ImU32 line_colour    = IM_COL32(255, 255, 255, 100);

constexpr ImU32 valid_background   = IM_COL32(0, 150, 255, 255);
constexpr ImU32 invalid_background = IM_COL32(255, 50, 50, 255);

const ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration
				      | ImGuiWindowFlags_AlwaysAutoResize
				      | ImGuiWindowFlags_NoNav;

const ImGuiTableFlags table_flags = ImGuiTableFlags_SizingFixedFit
				    | ImGuiTableFlags_BordersInnerV
				    | ImGuiTableFlags_RowBg;

ImU32 cell_background(bool initialised)
{
	return initialised ? valid_background : invalid_background;
}

void draw_size(const visual::Memory_Allocation &allocation)
{
	ImGui::TableNextColumn();
	std::string size = std::to_string(allocation.size());
	ImGui::TextUnformatted(size.c_str());
}

void draw_value(visual::Memory_Value const &value)
{
	ImGui::TableSetBgColor(
	    ImGuiTableBgTarget_CellBg,
	    cell_background(value.initialised()));

	ImGui::Text("%s", value.value().c_str());
}

// We use a table to represent multiple values in a allocation element. For
// example an allocation having elements with mutliple values can be represented
// in different rows as:
//
// Element 1: Value A, Element 2: Value A, Element 3: Value A,
//            Value B,            Value B,            Value B, ....
//            Value C,            Value C,            Value C,
//
// Using a single table was the best way found to do it because ImGui does not
// auto scale the cells in nested tables.
void draw_allocation(
    std::map<Address, Table_Value>  &table_values,
    const visual::Memory_Allocation &allocation)
{
	for (std::size_t i = 0; i < allocation.max_element_size(); ++i)
	{
		if (i != 0)
		{
			ImGui::TableNextRow();
		}

		ImGui::TableSetColumnIndex(0);

		for (auto const &element : allocation)
		{
			ImGui::TableNextColumn();

			if (element.size() <= i)
			{
				continue;
			}

			draw_value(element[i]);

			ImGuiContext *g                             = GImGui;
			table_values[element.address_of_element(i)] = Table_Value{
			    g->CurrentTable,
			    ImGui::TableGetColumnIndex(),
			    ImGui::TableGetRowIndex()};
		}
	}
}

std::map<Address, Table_Value> draw_values(const visual::Memory &memory)
{
	std::map<Address, Table_Value> table_values;
	for (auto const &allocation : memory)
	{
		bool              open = true;
		const std::string window_name =
		    fmt::format("Allocation_{}", allocation.address());

		ImGui::Begin(window_name.c_str(), &open, window_flags);
		ImGui::SetWindowFontScale(2.0F);

		ImGui::BeginTable(
		    "Elements",
		    1 + static_cast<int>(allocation.size()),
		    table_flags);

		draw_size(allocation);
		draw_allocation(table_values, allocation);

		ImGui::EndTable();

		ImGui::End();
	}
	return table_values;
}

enum class Link_Position
{
	Front,
	End
};

ImVec2 link_position(
    const std::map<Address, Table_Value> &table_values,
    Address                               address,
    Link_Position                         position)
{
	ImGuiTable const *table  = table_values.at(address).table;
	int const         column = table_values.at(address).column;
	auto const  row = static_cast<float>(table_values.at(address).row);
	float const middle_offset = 0.5F;

	float horizontal = 0.0F;
	switch (position)
	{
	case Link_Position::Front:
		horizontal = table->Columns[column].MinX;
		break;

	case Link_Position::End:
		horizontal = table->Columns[column].MaxX;
		break;
	}

	float row_middle = table->LastFirstRowHeight * (row + middle_offset);

	return ImVec2{horizontal, table->OuterRect.Min.y + row_middle};
}

void draw_pointer_links(
    const std::map<Address, Table_Value> &table_values,
    const visual::Memory                 &memory)
{
	for (auto const &allocation : memory)
	{
		Address element_address = allocation.address();
		for (auto const &element : allocation)
		{
			Address current_address = element_address;
			for (auto const &value : element)
			{
				if (value.is_pointer()
				    && value.pointee_address() != 0U)
				{
					ImGui::GetBackgroundDrawList()->AddLine(
					    link_position(
						table_values,
						current_address,
						Link_Position::End),
					    link_position(
						table_values,
						value.pointee_address(),
						Link_Position::Front),
					    line_colour,
					    line_thickness);
				}

				current_address += value.size();
			}
			element_address += allocation.element_size();
		}
	}
}
} // namespace

namespace visual
{

void Viewport::add_event(Event &&event)
{
	spdlog::trace("Added eventof type: {}", to_string(event));
	m_events.push_back(std::move(event));
}

void Viewport::update(std::chrono::microseconds delta_time)
{
	if (m_event_timeout.count() > 0)
	{
		m_event_timeout -= delta_time;
		return;
	}

	m_event_timeout = event_duration;

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
	const std::map<Address, Table_Value> table_values = draw_values(m_memory);
	draw_pointer_links(table_values, m_memory);
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
	m_memory.insert(
	    Memory_Allocation{event.address(), event.size(), event.element_size()});
	return true;
}

bool Viewport::process(const Deallocated_Array_Event &event)
{
	m_memory.erase(event.address());
	return true;
}

bool Viewport::process(const Copy_Assignment_Event &event)
{
	return update_value(event.address(), event.value());
}

bool Viewport::process(const Move_Assignment_Event &event)
{
	// This prevents short circuting
	const bool moved_to   = updated_moved_to_element(event);
	const bool moved_from = updated_moved_from_element(event);
	return moved_to || moved_from;
}

bool Viewport::process(const Swap_Event &event)
{
	const bool copied_lhs =
	    update_value(event.lhs_address(), event.lhs_value());

	const bool copied_rhs =
	    update_value(event.rhs_address(), event.rhs_value());

	return copied_lhs && copied_rhs;
}

bool Viewport::updated_moved_to_element(const Move_Assignment_Event &event)
{
	return update_value(event.to_address(), event.value());
}

bool Viewport::updated_moved_from_element(const Move_Assignment_Event &event)
{
	return update_element(
	    event.from_address(),
	    event.value().is_pointer()
		? Memory_Value(event.value().size(), false, 0U)
		: Memory_Value(event.value().size(), false, ""));
}

bool Viewport::update_value(Address address, const Memory_Value &value)
{
	bool success = update_element(address, value);
	if (!success)
	{
		spdlog::warn("Received an assignment event for an address outside the registered ranges");
	}
	return success;
}

bool Viewport::update_element(Address address, const Memory_Value &value)
{
	return m_memory.update_element(address, value);
}

} // namespace visual
