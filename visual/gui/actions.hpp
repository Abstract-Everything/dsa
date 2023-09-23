#ifndef VISUAL_ACTIONS_HPP
#define VISUAL_ACTIONS_HPP

#include "enable_event_registration_scope.hpp"
#include "templates.hpp"

#include <dsa/algorithms.hpp>
#include <dsa/type_detector.hpp>

#include <fmt/format.h>
#include <imgui.h>

#include <algorithm>
#include <random>
#include <sstream>
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
concept Is_Random_Access_Iterator = requires(T i, T j, size_t n) {
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

	int          m_random_size = 0;
	std::mt19937 m_rng{std::random_device{}()};

	void properties();
	void accessors();
	void modifiers();

	bool is_in_range(std::size_t index);
	bool is_last_index(std::size_t index);

	void section(char const *label, void (Actions_UI::*interface)());
	void index_input(char const *label, int &value, bool allow_end_index);
	bool conditional_button(char const *label, bool enabled);

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
	static constexpr bool has_indexed_insert = has_member_insert_v<Container, std::size_t, Value>;
	static constexpr bool has_insert = has_member_insert_v<Container, Value>;
	static constexpr bool has_push   = has_member_push_v<Container, Value>;
	static constexpr bool has_value_erase =
	    has_member_erase_v<Container, Value> && has_member_contains_v<Container, Value>;
	// We do not support a structure that has both value and index erase. We
	// did not find a way to detect if index erase is present when value
	// erase is. If we try to detect an index erase, an implicit cast could
	// construct a Value from a size_t, this will result in a false positive
	static constexpr bool has_indexed_erase =
	    has_member_erase_v<Container, std::size_t> && !has_value_erase;
	static constexpr bool has_pop = has_member_pop_v<Container>;

	template<typename Allocator>
	using Has_Iterator = typename Allocator::Iterator;

	template<typename Default_Type, typename Allocator>
	using Detect_Iterator_T = dsa::Detect_Default_T<Default_Type, Has_Iterator, Allocator>;

	static constexpr bool has_random_access_iterators =
	    Is_Random_Access_Iterator<Detect_Iterator_T<std::void_t<>, Container>>;
};

template<typename Container>
void Actions_UI<Container>::draw() {
	Enable_Event_Registration_Scope::block_events();
	section("Properties", &Actions_UI::properties);
	section("Accessors", &Actions_UI::accessors);
	section("Modifiers", &Actions_UI::modifiers);
}

template<typename Container>
void Actions_UI<Container>::properties() {
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
void Actions_UI<Container>::accessors() {
	if constexpr (has_front)
	{
		ImGui::LabelText(
		    "Front element",
		    "%s",
		    m_container.empty() ? "Container is empty"
					: element_value(m_container.front()).c_str());
	}

	if constexpr (has_back)
	{
		ImGui::LabelText(
		    "Back element",
		    "%s",
		    m_container.empty() ? "Container is empty"
					: element_value(m_container.back()).c_str());
	}

	if constexpr (has_top)
	{
		ImGui::LabelText(
		    "Top element",
		    "%s",
		    m_container.empty() ? "Container is empty"
					: element_value(m_container.top()).c_str());
	}

	if constexpr (has_operator_access)
	{
		ImGui::Separator();
		index_input("Index to read", m_read, false);
		auto const read = static_cast<std::size_t>(m_read);
		ImGui::LabelText(
		    "Value read",
		    "%s",
		    is_in_range(read) ? element_value(m_container[read]).c_str() : "<Out of bounds>");
	}
}

template<typename Container>
void Actions_UI<Container>::modifiers() {
	if constexpr (has_clear)
	{
		ImGui::Text("Remove all the elements from the container");
		ImGui::SameLine();
		if (ImGui::Button("Clear"))
		{
			Enable_Event_Registration_Scope scope;
			m_container.clear();
		}
	}

	if constexpr (has_shrink_to_fit)
	{
		ImGui::Text("Deallocate unused space");
		ImGui::SameLine();
		if (ImGui::Button("Shrink to fit"))
		{
			Enable_Event_Registration_Scope scope;
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
			Enable_Event_Registration_Scope scope;
			auto const                      size = static_cast<std::size_t>(m_resize);
			m_container.resize(size);
		}
	}

	if constexpr (has_operator_access)
	{
		ImGui::Separator();
		index_input("Index of element to write", m_write, false);
		ImGui::InputInt("Value to write", &m_write_value);

		auto const write = static_cast<std::size_t>(m_write);
		if (conditional_button("Write", is_in_range(write)))
		{
			Enable_Event_Registration_Scope scope;
			m_container[write] = Value{m_write_value};
		}
	}

	if constexpr (has_append)
	{
		ImGui::Separator();
		ImGui::InputInt("Append Value", &m_append_value);
		if (ImGui::Button("Append"))
		{
			Enable_Event_Registration_Scope scope;
			m_container.append(Value{m_append_value});
		}
	}

	if constexpr (has_indexed_insert)
	{
		ImGui::Separator();
		index_input("Index of element to insert", m_insert, true);
		ImGui::InputInt("Insert Value", &m_insert_value);

		auto const insert = static_cast<std::size_t>(m_insert);
		if (conditional_button("Indexed Insert", is_in_range(insert) || is_last_index(insert)))
		{
			Enable_Event_Registration_Scope scope;
			m_container.insert(insert, Value{m_insert_value});
		}
	}

	if constexpr (has_insert)
	{
		ImGui::Separator();
		ImGui::InputInt("Insert Value", &m_insert_value);

		if (ImGui::Button("Insert"))
		{
			Enable_Event_Registration_Scope scope;
			m_container.insert(Value{m_insert_value});
		}
	}

	if constexpr (has_push)
	{
		ImGui::Separator();
		ImGui::InputInt("Push Value", &m_insert_value);

		if (ImGui::Button("Push"))
		{
			Enable_Event_Registration_Scope scope;
			m_container.push(Value{m_insert_value});
		}
	}

	if constexpr (has_indexed_erase)
	{
		ImGui::Separator();

		index_input("Index of element to erase", m_erase, false);

		auto const erase = static_cast<std::size_t>(m_erase);
		if (conditional_button("Erase", is_in_range(erase)))
		{
			Enable_Event_Registration_Scope scope;
			m_container.erase(erase);
		}
	}

	if constexpr (has_value_erase)
	{
		ImGui::Separator();

		ImGui::InputInt("Erase Value", &m_erase_value);

		if (conditional_button("Erase", m_container.contains(m_erase_value)))
		{
			Enable_Event_Registration_Scope scope;
			m_container.erase(Value{m_erase_value});
		}
	}

	if constexpr (has_pop)
	{
		ImGui::Text("Remove the top element from the container");
		ImGui::SameLine();
		if (conditional_button("Pop", m_container.size() > 1))
		{
			Enable_Event_Registration_Scope scope;
			m_container.pop();
		}
	}

	if constexpr (has_append || has_insert || has_push)
	{
		ImGui::Separator();

		ImGui::InputInt("Fill the container with random elements", &m_random_size);
		if (ImGui::Button("Fill"))
		{
			m_random_size = std::max(0, m_random_size);
			std::uniform_int_distribution<int> distribution(-m_random_size, m_random_size);
			for (int i = 0; i < m_random_size; ++i)
			{
				Enable_Event_Registration_Scope scope;
				Value                           value{distribution(m_rng)};
				if constexpr (has_append)
				{
					m_container.append(value);
				}
				else if constexpr (has_insert)
				{
					m_container.insert(value);
				}
				else if constexpr (has_push)
				{
					m_container.push(value);
				}
			}
		}
	}

	if constexpr (has_random_access_iterators)
	{
		using std::swap;

		ImGui::Separator();

		// TODO: Write an actual algorithm for this
		ImGui::Text("Shuffle the elements of the container");
		ImGui::SameLine();
		if (ImGui::Button("Shuffle"))
		{
			std::uniform_int_distribution<size_t> distribution(0, m_container.size() - 1);
			for (size_t i = 0; i < m_container.size(); ++i)
			{
				Enable_Event_Registration_Scope scope;
				swap(m_container[i], m_container[distribution(m_rng)]);
			}
		}

		ImGui::Separator();

		ImGui::Text("Insertion sort");
		ImGui::SameLine();
		if (ImGui::Button("Insertion sort"))
		{
			Enable_Event_Registration_Scope scope;
			dsa::insertion_sort(m_container.begin(), m_container.end());
		}

		ImGui::Text("Selection sort");
		ImGui::SameLine();
		if (ImGui::Button("Selection sort"))
		{
			Enable_Event_Registration_Scope scope;
			dsa::selection_sort(m_container.begin(), m_container.end());
		}

		ImGui::Text("Merge sort");
		ImGui::SameLine();
		if (ImGui::Button("Merge sort"))
		{
			Enable_Event_Registration_Scope scope;
			dsa::merge_sort<decltype(m_container.begin()), typename Container::Allocator>(
			    m_container.begin(),
			    m_container.end());
		}
	}
}

template<typename Container>
bool Actions_UI<Container>::is_in_range(std::size_t index) {
	return m_container.size() > index;
}

template<typename Container>
bool Actions_UI<Container>::is_last_index(std::size_t index) {
	return m_container.size() == index;
}

template<typename Container>
void Actions_UI<Container>::section(char const *label, void (Actions_UI::*interface)()) {
	if (!ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen))
	{
		return;
	}

	ImGui::Indent();
	(this->*interface)();
	ImGui::Unindent();
}

template<typename Container>
void Actions_UI<Container>::index_input(char const *label, int &value, bool allow_end_index) {
	auto const size      = static_cast<int>(m_container.size());
	auto const max_index = std::max(0, allow_end_index ? size : size - 1);
	value                = std::clamp(value, 0, max_index);
	ImGui::SliderInt(label, &value, 0, max_index);
}

template<typename Container>
bool Actions_UI<Container>::conditional_button(char const *label, bool enabled) {
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
std::string Actions_UI<Container>::element_value(typename Container::Const_Reference value) {
	std::stringstream stream;
	stream << value;
	return stream.str();
}

} // namespace visual

#endif
