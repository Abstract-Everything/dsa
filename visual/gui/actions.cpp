#include "actions.hpp"

#include <fmt/format.h>
#include <imgui.h>

#include <algorithm>

namespace visual
{

void Actions_UI::draw()
{
	ImGui::Begin("Actions User Interface");

	section("Properties", &Actions_UI::properties);
	section("Accessors", &Actions_UI::accessors);
	section("Modifiers", &Actions_UI::modifiers);

	ImGui::End();
}

void Actions_UI::properties()
{
	ImGui::LabelText("Is empty", m_vector.empty() ? "true" : "false");

	ImGui::LabelText("Size", "%lu", m_vector.size());

	ImGui::LabelText("Capacity", "%lu", m_vector.capacity());
}

void Actions_UI::accessors()
{
	ImGui::LabelText(
	    "Front element",
	    "%s",
	    m_vector.empty() ? "Container is empty"
			     : m_vector.front().to_string().c_str());

	ImGui::LabelText(
	    "Back element",
	    "%s",
	    m_vector.empty() ? "Container is empty"
			     : m_vector.back().to_string().c_str());

	ImGui::Separator();

	{
		index_input("Index to read", &m_read);
		const auto  read  = static_cast<std::size_t>(m_read);
		std::string value = fmt::format(
		    "{}",
		    is_in_range(read) ? m_vector[read].to_string()
				      : "<Out of bounds>");
		ImGui::LabelText("Value read", "%s", value.c_str());
	}

	ImGui::Separator();

	{
		ImGui::InputInt("Value to write", &m_write_value);
		index_input("Index of element to write", &m_write);

		const auto write = static_cast<std::size_t>(m_write);
		if (button("Write", is_in_range(write)))
		{
			m_vector[write] = Element_Monitor<int>{ m_write_value };
		}
	}
}

void Actions_UI::modifiers()
{
	ImGui::Text("Remove all the elements from the container");
	ImGui::SameLine();
	if (ImGui::Button("Clear"))
	{
		m_vector.clear();
	}

	ImGui::Text("Deallocate unused space");
	ImGui::SameLine();
	if (ImGui::Button("Shrink to fit"))
	{
		m_vector.shrink_to_fit();
	}

	ImGui::Separator();

	{
	     ImGui::InputInt("Container size to set", &m_resize);
	     m_resize = std::max(0, m_resize);
	     if (ImGui::Button("Resize"))
	     {
	     	const auto size = static_cast<std::size_t>(m_resize);
	     	m_vector.resize(size);
	     }
	}

	ImGui::Separator();

	{
		ImGui::InputInt("Append Value", &m_append_value);
		if (ImGui::Button("Append"))
		{
			m_vector.append(Element_Monitor<int>{ m_append_value });
		}
	}

	ImGui::Separator();

	{
		ImGui::InputInt("Insert Index", &m_insert);
		ImGui::InputInt("Insert Value", &m_insert_value);

		const auto insert = static_cast<std::size_t>(m_insert);
		if (button("Insert", is_in_range(insert) || is_last_index(insert)))
		{
			m_vector.insert(
			    insert,
			    Element_Monitor<int>{ m_insert_value });
		}
	}

	ImGui::Separator();

	{
		ImGui::InputInt("Erase Index", &m_erase);

		const auto erase = static_cast<std::size_t>(m_erase);
		if (button("Erase", is_in_range(erase)))
		{
			m_vector.erase(erase);
		}
	}
}

bool Actions_UI::is_in_range(std::size_t index)
{
	return m_vector.size() > index;
}

bool Actions_UI::is_last_index(std::size_t index)
{
	return m_vector.size() == index;
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
	auto size = static_cast<int>(m_vector.size());
	ImGui::SliderInt(label, value, 0, std::max(0, size - 1));
}

bool Actions_UI::button(const char *label, bool enabled)
{
	if (!enabled)
	{
		ImGui::BeginDisabled();
	}

	bool clicked = ImGui::Button(label);

	if (!enabled)
	{
		ImGui::EndDisabled();
	}

	return clicked;
}

} // namespace visual
