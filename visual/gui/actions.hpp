#ifndef VISUAL_ACTIONS_HPP
#define VISUAL_ACTIONS_HPP

#include "templates.hpp"

#include <dsa/algorithms.hpp>
#include <dsa/type_detector.hpp>

#include <fmt/format.h>
#include <imgui.h>

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

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
DEFINE_HAS_MEMBER(contains);
DEFINE_HAS_MEMBER(pop);
DEFINE_HAS_OPERATOR_ACCESS();

// TODO: Use std::random_access_iterator concepts
template<typename T>
concept Is_Random_Access_Iterator = requires(T i, T j, size_t n)
{
    { i + n };
    { i - n };
    { i == j };
    { i != j };
    { --i };
    { ++i };
};

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

	// If we read the container state using its methods it fires events to
	// the viewport. Since we access this state every frame a lot of
	// unneeded events are created. We keep a cache of the state in order to
	// avoid this.
	bool                     m_cached_empty    = true;
	size_t                   m_cached_size     = 0;
	size_t                   m_cached_capacity = 0;
	std::string              m_cached_front;
	std::string              m_cached_back;
	std::string              m_cached_top;
	std::vector<std::string> m_cached_values;

	void properties();
	void accessors();
	void modifiers();

	void refresh_cached_values();

	bool is_in_range(std::size_t index);
	bool is_last_index(std::size_t index);

	void section(const char *label, void (Actions_UI::*interface)());
	void index_input(const char *label, int &value, bool allow_end_index);
	bool conditional_button(const char *label, bool enabled);

	std::string element_value(typename Container::Const_Reference value);

	static constexpr bool has_empty           = has_member_empty_v<Container>;
	static constexpr bool has_size            = has_member_size_v<Container>;
	static constexpr bool has_capacity        = has_member_capacity_v<Container>;
	static constexpr bool has_front           = has_member_front_v<Container>;
	static constexpr bool has_back            = has_member_back_v<Container>;
	static constexpr bool has_top             = has_member_top_v<Container>;
	static constexpr bool has_operator_access = has_operator_access_v<Container, std::size_t>;
	static constexpr bool has_clear           = has_member_clear_v<Container>;
	static constexpr bool has_shrink_to_fit   = has_member_shrink_to_fit_v<Container>;
	static constexpr bool has_resize          = has_member_resize_v<Container, std::size_t>;
	static constexpr bool has_append          = has_member_append_v<Container, Value>;
	static constexpr bool has_indexed_insert  = has_member_insert_v<Container, std::size_t, Value>;
	static constexpr bool has_insert          = has_member_insert_v<Container, Value>;
	static constexpr bool has_push            = has_member_push_v<Container, Value>;
	static constexpr bool has_value_erase     = has_member_erase_v<Container, Value> && has_member_contains_v<Container, Value>;
	// We do not support a structure that has both value and index erase. We
	// did not find a way to detect if index erase is present when value
	// erase is. If we try to detect an index erase, an implicit cast could
	// construct a Value from a size_t, this will result in a false positive
	static constexpr bool has_indexed_erase   = has_member_erase_v<Container, std::size_t> && !has_value_erase;
	static constexpr bool has_pop             = has_member_pop_v<Container>;

	template<typename Allocator>
	using Has_Iterator = typename Allocator::Iterator;

	template<typename Default_Type, typename Allocator>
	using Detect_Iterator_T =
	    dsa::Detect_Default_T<Default_Type, Has_Iterator, Allocator>;

	static constexpr bool has_random_access_iterators =
	    Is_Random_Access_Iterator<Detect_Iterator_T<std::void_t<>, Container>>;
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
		ImGui::LabelText("Is empty", m_cached_empty ? "true" : "false");
	}

	if constexpr (has_size)
	{
		ImGui::LabelText("Size", "%lu", m_cached_size);
	}

	if constexpr (has_capacity)
	{
		ImGui::LabelText("Capacity", "%lu", m_cached_capacity);
	}
}

template<typename Container>
void Actions_UI<Container>::accessors()
{
	if constexpr (has_front)
	{
		ImGui::LabelText("Front element", "%s", m_cached_front.c_str());
	}

	if constexpr (has_back)
	{
		ImGui::LabelText("Back element", "%s", m_cached_back.c_str());
	}

	if constexpr (has_top)
	{
		ImGui::LabelText("Top element", "%s", m_cached_top.c_str());
	}

	if constexpr (has_operator_access)
	{
		ImGui::Separator();
		index_input("Index to read", m_read, false);
		const auto  read  = static_cast<std::size_t>(m_read);
		std::string value = is_in_range(read) ? m_cached_values[read]
						      : "<Out of bounds>";

		ImGui::LabelText("Value read", "%s", value.c_str());
	}
}

template<typename Container>
void Actions_UI<Container>::modifiers()
{
	bool modified = false;
	if constexpr (has_clear)
	{
		ImGui::Text("Remove all the elements from the container");
		ImGui::SameLine();
		if (ImGui::Button("Clear"))
		{
			m_container.clear();
			modified = true;
		}
	}

	if constexpr (has_shrink_to_fit)
	{
		ImGui::Text("Deallocate unused space");
		ImGui::SameLine();
		if (ImGui::Button("Shrink to fit"))
		{
			m_container.shrink_to_fit();
			modified = true;
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
			modified = true;
		}
	}

	if constexpr (has_operator_access)
	{
		ImGui::Separator();
		index_input("Index of element to write", m_write, false);
		ImGui::InputInt("Value to write", &m_write_value);

		const auto write = static_cast<std::size_t>(m_write);
		if (conditional_button("Write", is_in_range(write)))
		{
			m_container[write] = Value{m_write_value};
		}
	}

	if constexpr (has_append)
	{
		ImGui::Separator();
		ImGui::InputInt("Append Value", &m_append_value);
		if (ImGui::Button("Append"))
		{
			m_container.append(Value{m_append_value});
			modified = true;
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
			modified = true;
		}
	}

	if constexpr (has_insert)
	{
		ImGui::Separator();
		ImGui::InputInt("Insert Value", &m_insert_value);

		if (ImGui::Button("Insert"))
		{
			m_container.insert(Value{m_insert_value});
			modified = true;
		}
	}

	if constexpr (has_push)
	{
		ImGui::Separator();
		ImGui::InputInt("Push Value", &m_insert_value);

		if (ImGui::Button("Push"))
		{
			m_container.push(Value{m_insert_value});
			modified = true;
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
			modified = true;
		}
	}

	if constexpr (has_value_erase)
	{
		ImGui::Separator();

		ImGui::InputInt("Erase Value", &m_erase_value);

		Value      erase_value{m_erase_value};
		const bool contains_value = std::find(
						m_cached_values.begin(),
						m_cached_values.end(),
						element_value(erase_value))
					    != m_cached_values.end();

		if (conditional_button("Erase", contains_value))
		{
			m_container.erase(erase_value);
			modified = true;
		}
	}

	if constexpr (has_pop)
	{
		ImGui::Text("Remove the top element from the container");
		ImGui::SameLine();
		if (ImGui::Button("Pop"))
		{
			m_container.pop();
			modified = true;
		}
	}

	if constexpr (has_random_access_iterators)
	{
		ImGui::Text("Insertion sort");
		ImGui::SameLine();
		if (ImGui::Button("Insertion sort"))
		{
			dsa::insertion_sort(
			    m_container.begin(),
			    m_container.end(),
			    std::less{});
			modified = true;
		}
	}

	if (modified)
	{
		refresh_cached_values();
	}
}

template<typename Container>
void Actions_UI<Container>::refresh_cached_values()
{
	if constexpr (has_empty)
	{
		m_cached_empty = m_container.empty();
	}

	if constexpr (has_size)
	{
		m_cached_size = m_container.size();
	}

	if constexpr (has_capacity)
	{
		m_cached_capacity = m_container.capacity();
	}

	if constexpr (has_front)
	{
		m_cached_front = m_cached_empty
				     ? "Container is empty"
				     : element_value(m_container.front());
	}

	if constexpr (has_back)
	{
		m_cached_back = m_cached_empty
				    ? "Container is empty"
				    : element_value(m_container.back());
	}

	if constexpr (has_top)
	{
		m_cached_back = m_cached_empty
				    ? "Container is empty"
				    : element_value(m_container.top());
	}

	m_cached_values.clear();
	if constexpr (has_operator_access || has_value_erase)
	{
		for (auto value : m_container)
		{
			m_cached_values.push_back(element_value(value));
		}
	}
}

template<typename Container>
bool Actions_UI<Container>::is_in_range(std::size_t index)
{
	return m_cached_size > index;
}

template<typename Container>
bool Actions_UI<Container>::is_last_index(std::size_t index)
{
	return m_cached_size == index;
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
	const auto size      = static_cast<int>(m_cached_size);
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

template<typename Container>
std::string Actions_UI<Container>::element_value(
    typename Container::Const_Reference value)
{
	std::stringstream stream;
	stream << value;
	return stream.str();
}

} // namespace visual

#endif
