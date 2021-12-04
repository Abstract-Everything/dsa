#include "actions.hpp"

#include <imgui.h>

#include <algorithm>

namespace visual
{

void Actions_UI::draw()
{
	ImGui::Begin("Actions User Interface");

	section("Properties", &Actions_UI::properties);
	section("Read value", &Actions_UI::read);
	section("Resize array", &Actions_UI::resize);
	section("Write value at index", &Actions_UI::write);

	ImGui::End();
}

void Actions_UI::properties()
{
	ImGui::LabelText("Array size", "%lu", m_dynamic_array.size());
}

void Actions_UI::read()
{
	index_input("Index to read", &m_read);

	const auto  read  = static_cast<std::size_t>(m_read);
	std::string value = "<Out of bounds>";
	if (is_in_range(read))
	{
		value = m_dynamic_array[read].to_string();
	}

	ImGui::LabelText("Value read", "%s", value.c_str());
}

void Actions_UI::resize()
{
	ImGui::InputInt("Array size to set", &m_resize);
	m_resize = std::max(0, m_resize);
	if (ImGui::Button("Resize"))
	{
		const auto size = static_cast<std::size_t>(m_resize);
		m_dynamic_array.resize(size);
	}
}

void Actions_UI::write()
{
	ImGui::InputInt("Value to write", &m_value);
	index_input("Index to write to", &m_write);

	const auto write = static_cast<std::size_t>(m_write);
	if (!is_in_range(write))
	{
		ImGui::BeginDisabled();
	}

	if (ImGui::Button("Write"))
	{
		m_dynamic_array[write] = Element_Monitor<int>{ m_value };
	}

	if (!is_in_range(write))
	{
		ImGui::EndDisabled();
	}
}

bool Actions_UI::is_in_range(std::size_t index)
{
	return m_dynamic_array.size() > index;
}

void Actions_UI::section(const char *label, void (Actions_UI::*interface)())
{
	if (!ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen))
	{
		return;
	}

	ImGui::Indent();

	(this->*interface)();

	ImGui::Unindent();
}

void Actions_UI::index_input(const char *label, int *value)
{
	auto size = static_cast<int>(m_dynamic_array.size());
	ImGui::SliderInt(label, value, 0, std::max(0, size - 1));
}

} // namespace visual
