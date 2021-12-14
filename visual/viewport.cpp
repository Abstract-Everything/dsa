#include "viewport.hpp"

#include <imgui.h>
#include <spdlog/spdlog.h>

#include <exception>

namespace
{

constexpr std::chrono::seconds event_duration{ 2 };
constexpr ImU32                valid_background   = IM_COL32(0, 150, 255, 255);
constexpr ImU32                invalid_background = IM_COL32(255, 50, 50, 255);

void draw_buffer(const visual::Buffer &buffer)
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

	ImGui::TableNextColumn();
	std::string size = std::to_string(buffer.size());
	ImGui::TextUnformatted(size.c_str());

	for (auto const &element : buffer)
	{
		const std::string_view value = element.value();
		ImU32 background = element.initialised() ? valid_background
							 : invalid_background;

		ImGui::TableNextColumn();
		ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, background);
		ImGui::TextUnformatted(value.data(), value.data() + value.length());
	}
	ImGui::EndTable();

	ImGui::End();
}

} // namespace

namespace visual
{

void Viewport::add_event(Event &&event)
{
	spdlog::trace("Added eventof type: {}", typeid(event).name());
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
	for (auto const &buffer : m_buffers)
	{
		draw_buffer(buffer);
	}
}

bool Viewport::process(const Event &event)
{
	spdlog::trace("Processing event of type: {}", typeid(event).name());

	return std::visit(
	    [this](auto &&event_typed) { return process(event_typed); },
	    event);
}

bool Viewport::process(const Allocated_Array_Event &event)
{
	Buffer buffer{ event.address(), event.size(), event.element_size() };

	if (std::find_if(
		m_buffers.begin(),
		m_buffers.end(),
		[&buffer](const Buffer &other)
		{ return Buffer::overlap(other, buffer); })
	    != m_buffers.end())
	{
		spdlog::warn("Received a duplicated allocate event");
		return false;
	}

	m_buffers.push_back(buffer);

	return true;
}

bool Viewport::process(const Deallocated_Array_Event &event)
{
	auto buffer_it = std::find_if(
	    m_buffers.begin(),
	    m_buffers.end(),
	    [&event](const Buffer &buffer)
	    { return buffer.contains(event.address()); });

	if (buffer_it == m_buffers.end())
	{
		spdlog::warn(
		    "Received a deallocate event on a non monitored address");
		return false;
	}

	m_buffers.erase(buffer_it);

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
	return update_element(event.from_address(), Memory_Value());
}

bool Viewport::update_element(
    std::string_view    log_message,
    std::uint64_t       address,
    const Memory_Value &value)
{
	bool success = update_element(address, value);
	if (!success)
	{
		spdlog::warn(log_message);
	}
	return success;
}

bool Viewport::update_element(std::uint64_t address, const Memory_Value &value)
{
	auto buffer_it = std::find_if(
	    m_buffers.begin(),
	    m_buffers.end(),
	    [address](const Buffer &buffer) { return buffer.contains(address); });

	if (buffer_it >= m_buffers.end())
	{
		return false;
	}

	auto element_it =
	    buffer_it->begin()
	    + static_cast<std::ptrdiff_t>(buffer_it->index_of(address));

	if (element_it >= buffer_it->end())
	{
		spdlog::error("Tried to update an element outside of a buffer");
		return false;
	}

	*element_it = value;

	return true;
}

} // namespace visual
