#ifndef VISUAL_ACTIONS_HPP
#define VISUAL_ACTIONS_HPP

#include "templates.hpp"

#include <fmt/format.h>
#include <imgui.h>

#include <string>

namespace visual
{

DEFINE_HAS_MEMBER(empty);
DEFINE_HAS_MEMBER(size);
DEFINE_HAS_MEMBER(capacity);
DEFINE_HAS_MEMBER_OVERLOADED(front);
DEFINE_HAS_MEMBER_OVERLOADED(back);
DEFINE_HAS_MEMBER(clear);
DEFINE_HAS_MEMBER(shrink_to_fit);
DEFINE_HAS_MEMBER(resize);
DEFINE_HAS_MEMBER(append);
DEFINE_HAS_MEMBER(insert);
DEFINE_HAS_MEMBER(erase);

template<typename Container>
class Actions_UI
{
	using Value = typename Container::Value;

 public:
	void draw();

 private:
	Container m_container;

	int         m_read = 0;
	std::string m_read_value;

	int m_resize = 0;

	int m_write_value = 0;
	int m_write       = 0;

	int m_insert_value = 0;
	int m_insert       = 0;

	int m_append_value = 0;

	int m_erase = 0;

	void properties();
	void accessors();
	void modifiers();

	bool is_in_range(std::size_t index);
	bool is_last_index(std::size_t index);

	void section(const char *label, void (Actions_UI::*interface)());
	void index_input(const char *label, int &value, bool allow_end_index);
	bool conditional_button(const char *label, bool enabled);
};

template<typename Container>
void Actions_UI<Container>::draw()
{
	section("Properties", &Actions_UI::properties);
	section("Accessors", &Actions_UI::accessors);
	section("Modifiers", &Actions_UI::modifiers);
}

template<typename Container>
void Actions_UI<Container>::properties()
{
	if constexpr (has_member_empty_v<Container>)
	{
		ImGui::LabelText("Is empty", m_container.empty() ? "true" : "false");
	}

	if constexpr (has_member_size_v<Container>)
	{
		ImGui::LabelText("Size", "%lu", m_container.size());
	}

	if constexpr (has_member_capacity_v<Container>)
	{
		ImGui::LabelText("Capacity", "%lu", m_container.capacity());
	}
}

template<typename Container>
void Actions_UI<Container>::accessors()
{
	if constexpr (has_member_front_v<Container>)
	{
		ImGui::LabelText(
		    "Front element",
		    "%s",
		    m_container.empty()
			? "Container is empty"
			: m_container.front().to_string().c_str());
	}

	if constexpr (has_member_back_v<Container>)
	{
		ImGui::LabelText(
		    "Back element",
		    "%s",
		    m_container.empty() ? "Container is empty"
					: m_container.back().to_string().c_str());
	}

	if constexpr (has_member_operator_access_v<Container>)
	{
		ImGui::Separator();
		index_input("Index to read", m_read, false);
		const auto  read  = static_cast<std::size_t>(m_read);
		std::string value = fmt::format(
		    "{}",
		    is_in_range(read) ? m_container[read].to_string()
				      : "<Out of bounds>");
		ImGui::LabelText("Value read", "%s", value.c_str());

		ImGui::Separator();
		index_input("Index of element to write", m_write, false);
		ImGui::InputInt("Value to write", &m_write_value);

		const auto write = static_cast<std::size_t>(m_write);
		if (conditional_button("Write", is_in_range(write)))
		{
			m_container[write] = Value{ m_write_value };
		}
	}
}

template<typename Container>
void Actions_UI<Container>::modifiers()
{
	if constexpr (has_member_clear_v<Container>)
	{
		ImGui::Text("Remove all the elements from the m_container");
		ImGui::SameLine();
		if (ImGui::Button("Clear"))
		{
			m_container.clear();
		}
	}

	if constexpr (has_member_shrink_to_fit_v<Container>)
	{
		ImGui::Text("Deallocate unused space");
		ImGui::SameLine();
		if (ImGui::Button("Shrink to fit"))
		{
			m_container.shrink_to_fit();
		}
	}

	if constexpr (has_member_resize_v<Container>)
	{
		ImGui::Separator();
		ImGui::InputInt("Container size to set", &m_resize);
		m_resize = std::max(0, m_resize);
		if (ImGui::Button("Resize"))
		{
			const auto size = static_cast<std::size_t>(m_resize);
			m_container.resize(size);
		}
	}

	if constexpr (has_member_append_v<Container>)
	{
		ImGui::Separator();
		ImGui::InputInt("Append Value", &m_append_value);
		if (ImGui::Button("Append"))
		{
			m_container.append(Value{ m_append_value });
		}
	}

	if constexpr (has_member_insert_v<Container>)
	{
		ImGui::Separator();
		index_input("Index of element to insert", m_insert, true);
		ImGui::InputInt("Insert Value", &m_insert_value);

		const auto insert = static_cast<std::size_t>(m_insert);
		if (conditional_button(
			"Insert",
			is_in_range(insert) || is_last_index(insert)))
		{
			m_container.insert(insert, Value{ m_insert_value });
		}
	}

	if constexpr (has_member_erase_v<Container>)
	{
		ImGui::Separator();

		index_input("Index of element to erase", m_erase, false);

		const auto erase = static_cast<std::size_t>(m_erase);
		if (conditional_button("Erase", is_in_range(erase)))
		{
			m_container.erase(erase);
		}
	}
}

template<typename Container>
bool Actions_UI<Container>::is_in_range(std::size_t index)
{
	return m_container.size() > index;
}

template<typename Container>
bool Actions_UI<Container>::is_last_index(std::size_t index)
{
	return m_container.size() == index;
}

template<typename Container>
void Actions_UI<Container>::section(const char *label, void (Actions_UI::*interface)())
{
	if (!ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen))
	{
		return;
	}

	ImGui::Indent();
	(this->*interface)();
	ImGui::Unindent();
}

template<typename Container>
void Actions_UI<Container>::index_input(const char *label, int &value, bool allow_end_index)
{
	const auto size      = static_cast<int>(m_container.size());
	const auto max_index = std::max(0, allow_end_index ? size : size - 1);
	value                = std::clamp(value, 0, max_index);
	ImGui::SliderInt(label, &value, 0, max_index);
}

template<typename Container>
bool Actions_UI<Container>::conditional_button(const char *label, bool enabled)
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

#endif
