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
constexpr std::chrono::seconds event_duration{1};
} // namespace

namespace visual
{

void Viewport::update(std::chrono::microseconds delta_time) {
	if (m_event_timeout.count() > 0)
	{
		m_event_timeout -= delta_time;
		return;
	}

	m_event_timeout = event_duration;
	m_memory.update();
}

void Viewport::draw() {
	m_memory.draw();
}

} // namespace visual
