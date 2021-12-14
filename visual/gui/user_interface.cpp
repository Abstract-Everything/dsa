#include "user_interface.hpp"

#include "templates.hpp"

#include <array>

namespace
{

constexpr std::array<const char *, 3> structures{ "Empty", "Array", "Vector" };

} // namespace

namespace visual
{

void User_Interface::draw()
{
	if (!ImGui::CollapsingHeader(
		"Data structure operations",
		ImGuiTreeNodeFlags_DefaultOpen))
	{
		return;
	}

	ImGui::Indent();

	if (ImGui::CollapsingHeader(
		"Data structure selection",
		ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Indent();

		ImGui::Combo(
		    "Data structure type",
		    &m_selected_structure,
		    structures.data(),
		    static_cast<int>(structures.size()));

		auto selection = static_cast<std::size_t>(m_selected_structure);
		if (m_actions.index() != selection)
		{
			construct_variant_by_index(m_actions, selection);
		}

		ImGui::Unindent();
	}

	std::visit(
	    [](auto &&actions_ui)
	    {
		    if constexpr (!std::is_same_v<
				      std::decay_t<decltype(actions_ui)>,
				      std::monostate>)
		    {
			    actions_ui.draw();
		    }
	    },
	    m_actions);

	ImGui::Unindent();
}

} // namespace visual
