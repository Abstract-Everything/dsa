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
DEFINE_HAS_MEMBER(front);
DEFINE_HAS_MEMBER(back);
DEFINE_HAS_MEMBER(top);
DEFINE_HAS_MEMBER(resize);
DEFINE_HAS_MEMBER(clear);
DEFINE_HAS_MEMBER(shrink_to_fit);
DEFINE_HAS_MEMBER(append);
DEFINE_HAS_MEMBER(insert);
DEFINE_HAS_MEMBER(push);
DEFINE_HAS_MEMBER(erase);
DEFINE_HAS_MEMBER(pop);
DEFINE_HAS_OPERATOR_ACCESS();

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

	int m_erase_value = 0;
	int m_erase       = 0;

	void properties();
	void accessors();
	void modifiers();

	bool is_in_range(std::size_t index);
	bool is_last_index(std::size_t index);

	void section(const char *label, void (Actions_UI::*interface)());
	void index_input(const char *label, int &value, bool allow_end_index);
	bool conditional_button(const char *label, bool enabled);

	static constexpr bool has_empty           = has_member_empty_v<Container>;
	static constexpr bool has_size            = has_member_size_v<Container>;
	static constexpr bool has_capacity        = has_member_capacity_v<Container>;
	static constexpr bool has_front           = has_member_front_v<Container>;
	static constexpr bool has_back            = has_member_back_v<Container>;
	static constexpr bool has_top             = has_member_top_v<Container>;
	static constexpr bool has_operator_access = has_member_operator_access_v<Container, std::size_t>;
	static constexpr bool has_clear           = has_member_clear_v<Container>;
	static constexpr bool has_shrink_to_fit   = has_member_shrink_to_fit_v<Container>;
	static constexpr bool has_resize          = has_member_resize_v<Container, std::size_t>;
	static constexpr bool has_append          = has_member_append_v<Container, Value>;
	static constexpr bool has_indexed_insert  = has_member_insert_v<Container, std::size_t, Value>;
	static constexpr bool has_insert          = has_member_insert_v<Container, Value>;
	static constexpr bool has_push            = has_member_push_v<Container, Value>;
	static constexpr bool has_indexed_erase   = has_member_erase_v<Container, std::size_t>;
	static constexpr bool has_erase           = has_member_erase_v<Container, Value>;
	static constexpr bool has_pop             = has_member_pop_v<Container>;
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
	if constexpr (has_empty)
	{
		ImGui::LabelText("Is empty", m_container.empty() ? "true" : "false");
	}

	if constexpr (has_size)
	{
		ImGui::LabelText("Size", "%lu", m_container.size());
	}

	if constexpr (has_capacity)
	{
		ImGui::LabelText("Capacity", "%lu", m_container.capacity());
	}
}

template<typename Container>
void Actions_UI<Container>::accessors()
{
	if constexpr (has_front)
	{
		ImGui::LabelText(
		    "Front element",
		    "%s",
		    m_container.empty()
			? "Container is empty"
			: m_container.front().to_string().c_str());
	}

	if constexpr (has_back)
	{
		ImGui::LabelText(
		    "Back element",
		    "%s",
		    m_container.empty() ? "Container is empty"
					: m_container.back().to_string().c_str());
	}

	if constexpr (has_top)
	{
		ImGui::LabelText(
		    "Top element",
		    "%s",
		    m_container.empty() ? "Container is empty"
					: m_container.top().to_string().c_str());
	}

	if constexpr (has_operator_access)
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
			m_container[write] = Value{m_write_value};
		}
	}
}

template<typename Container>
void Actions_UI<Container>::modifiers()
{
	if constexpr (has_clear)
	{
		ImGui::Text("Remove all the elements from the m_container");
		ImGui::SameLine();
		if (ImGui::Button("Clear"))
		{
			m_container.clear();
		}
	}

	if constexpr (has_shrink_to_fit)
	{
		ImGui::Text("Deallocate unused space");
		ImGui::SameLine();
		if (ImGui::Button("Shrink to fit"))
		{
			m_container.shrink_to_fit();
		}
	}

	if constexpr (has_resize)
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

	if constexpr (has_append)
	{
		ImGui::Separator();
		ImGui::InputInt("Append Value", &m_append_value);
		if (ImGui::Button("Append"))
		{
			m_container.append(Value{m_append_value});
		}
	}

	if constexpr (has_indexed_insert)
	{
		ImGui::Separator();
		index_input("Index of element to insert", m_insert, true);
		ImGui::InputInt("Insert Value", &m_insert_value);

		const auto insert = static_cast<std::size_t>(m_insert);
		if (conditional_button(
			"Indexed Insert",
			is_in_range(insert) || is_last_index(insert)))
		{
			m_container.insert(insert, Value{m_insert_value});
		}
	}

	if constexpr (has_insert)
	{
		ImGui::Separator();
		ImGui::InputInt("Insert Value", &m_insert_value);

		if (ImGui::Button("Insert"))
		{
			m_container.insert(Value{m_insert_value});
		}
	}

	if constexpr (has_push)
	{
		ImGui::Separator();
		ImGui::InputInt("Push Value", &m_insert_value);

		if (ImGui::Button("Push"))
		{
			m_container.push(Value{m_insert_value});
		}
	}

	if constexpr (has_indexed_erase)
	{
		ImGui::Separator();

		index_input("Index of element to erase", m_erase, false);

		const auto erase = static_cast<std::size_t>(m_erase);
		if (conditional_button("Erase", is_in_range(erase)))
		{
			m_container.erase(erase);
		}
	}

	if constexpr (has_erase)
	{
		ImGui::Separator();

		ImGui::InputInt("Erase Value", &m_erase_value);

		Value erase_value{m_erase_value};
		if (conditional_button("Erase", m_container.contains(erase_value)))
		{
			m_container.erase(erase_value);
		}
	}

	if constexpr (has_pop)
	{
		ImGui::Text("Remove the top element from the container");
		ImGui::SameLine();
		if (ImGui::Button("Pop"))
		{
			m_container.pop();
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
