#include "viewport.hpp"

#include <dsa/memory_representation.hpp>

#include <imgui.h>
#include <imgui_internal.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <charconv>
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

ImU32 cell_background(bool initialised) {
	return initialised ? valid_background : invalid_background;
}

auto allocation_name(uintptr_t address) -> std::string {
	return fmt::format("Allocation_{:0x}", address);
}

ImGuiWindow *allocation_window(
    dsa::Memory_Representation const &memory,
    uintptr_t                         address) {
	uintptr_t source = 0;
	for (auto const &allocation : memory.allocations())
	{
		if (allocation->contains(address))
		{
			source = allocation->address();
			break;
		}
	}

	for (auto const &window : GImGui->Windows)
	{
		if (window->Name == allocation_name(source))
		{
			return window;
		}
	}
	return nullptr;
}

} // namespace

namespace visual
{

void Viewport::update(std::chrono::microseconds delta_time) {
	if (m_memory_representaion.size() <= 1)
	{
		return;
	}

	if (m_event_timeout.count() > 0)
	{
		m_event_timeout -= delta_time;
		return;
	}

	m_event_timeout = event_duration;
	m_memory_representaion.pop_front();
}

void Viewport::draw() const {
	draw_allocations();
	draw_nullptr();
	draw_pointers();
}

dsa::Memory_Representation const &Viewport::current() const {
	return m_memory_representaion.front();
}

void Viewport::draw_allocations() const {
	for (auto const &allocation : current().allocations())
	{
		if (allocation->owns_allocation())
		{
			draw_allocation(*allocation);
		}
	}
}

void Viewport::draw_allocation(dsa::Allocation_Block const &block) const {
	const std::string window_name = allocation_name(block.address());

	ImGui::Begin(window_name.c_str(), nullptr, window_flags);
	ImGui::SetWindowFontScale(2.0F);

	ImGui::BeginTable(
	    "Elements",
	    1 + static_cast<int>(block.count()),
	    table_flags);

	ImGui::TableNextColumn();
	std::string size = std::to_string(block.count());
	ImGui::TextUnformatted(size.c_str());

	for (auto const &field : block.fields())
	{
		draw_value(*field);
	}

	ImGui::EndTable();

	ImGui::End();
}

void Viewport::draw_value(dsa::Allocation_Element const &element) {
	if (!element.leaf())
	{
		for (auto const &field : element.fields())
		{
			draw_value(*field);
		}
		return;
	}

	if (element.pointer())
	{
		return;
	}

	ImGui::TableNextColumn();

	ImGui::TableSetBgColor(
	    ImGuiTableBgTarget_CellBg,
	    cell_background(element.initialised()));

	ImGui::Text("%s", element.value().c_str());
}

void Viewport::draw_nullptr() {
	const std::string window_name =
	    allocation_name(dsa::numeric_address(static_cast<void *>(nullptr)));
	ImGui::Begin(window_name.c_str(), nullptr, window_flags);
	ImGui::SetWindowFontScale(2.0F);
	ImGui::Text("null pointer");
	ImGui::End();
}

void Viewport::draw_pointers() const {
	for (auto const &allocation : current().allocations())
	{
		if (allocation->owns_allocation())
		{
			for (auto const &inner_element : allocation->fields())
			{
				draw_pointer(*inner_element);
			}
		}
	}
}

void Viewport::draw_pointer(dsa::Allocation_Element const &element) const {
	if (!element.leaf())
	{
		for (auto const &field : element.fields())
		{
			draw_pointer(*field);
		}
		return;
	}

	if (!element.pointer())
	{
		return;
	}

	ImGuiWindow *source = allocation_window(current(), element.address());
	assert(source != nullptr && "");

	uintptr_t    pointee     = pointer_value_address(element);
	ImGuiWindow *destination = allocation_window(current(), pointee);
	assert(destination != nullptr && "");

	ImGui::GetBackgroundDrawList()->AddLine(
	    source->Pos,
	    destination->Pos,
	    line_colour,
	    line_thickness);
}

auto Viewport::pointer_value_address(dsa::Allocation_Element const &element)
    -> uintptr_t {
	uintptr_t              address = 0;
	std::string            string  = element.value();
	std::from_chars_result result  = std::from_chars(
            // Skip the 0x in hexadecimal
            string.data() + 2,
            string.data() + string.length(),
            address,
            16); // hexadecimal base
	return result.ec == std::errc() ? address : 0;
}

} // namespace visual
