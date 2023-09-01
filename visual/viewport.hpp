#ifndef VISUAL_VIEWPORT_HPP
#define VISUAL_VIEWPORT_HPP

#include "memory.hpp"
#include "utilities/formatters.hpp"

#include <dsa/memory_representation.hpp>

#include <spdlog/spdlog.h>

#include <chrono>
#include <list>

namespace visual
{

class Viewport
{
 public:
	void add_event(dsa::Memory_Monitor_Event auto &&event) {
		spdlog::trace("Added eventof type: {}", event);
		m_memory.push(std::forward<decltype(event)>(event));
	}

	void update(std::chrono::microseconds delta_time);

	void draw();

 private:
	components::Memory m_memory;

	std::chrono::microseconds m_event_timeout{-1};

	[[nodiscard]] dsa::Memory_Representation const &current() const;

	void        draw_allocations() const;
	void        draw_allocation(dsa::Allocation_Block const &block) const;
	static void draw_value(dsa::Allocation_Element const &element);
	static void draw_nullptr();
	void        draw_pointers() const;
	void        draw_pointer(dsa::Allocation_Element const &element) const;

	static auto pointer_value_address(dsa::Allocation_Element const &element) -> uintptr_t;
};

} // namespace visual

#endif
