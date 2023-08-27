#ifndef VISUAL_MONITOR_HPP
#define VISUAL_MONITOR_HPP

#include "main_window.hpp"

#include <dsa/memory_monitor.hpp>

#include <sstream>

namespace visual
{

class Visual_Monitor
{
 public:
	template<typename T>
	static auto before_deallocate(dsa::Allocation_Event<T> /* event */) -> bool {
		return true;
	}

	template<typename T>
	static void process_allocation_event(dsa::Allocation_Event<T> event) {
		Main_Window::add_event(std::move(event));
	}

	template<typename T>
	static void process_object_event(dsa::Object_Event<T> event) {
		Main_Window::add_event(std::move(event));
	}
};

} // namespace visual

#endif
