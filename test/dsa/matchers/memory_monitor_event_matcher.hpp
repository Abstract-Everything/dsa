#ifndef TEST_DSA_STATIC_MEMORY_MONITOR_EVENT_MATCHER_HPP
#define TEST_DSA_STATIC_MEMORY_MONITOR_EVENT_MATCHER_HPP

#include "utilities/memory_monitor_event_handler.hpp"

#include <catch2/matchers/catch_matchers_templated.hpp>

namespace test
{

struct EqualsMemoryMonitorEventMatcher : Catch::Matchers::MatcherGenericBase
{
	explicit EqualsMemoryMonitorEventMatcher(Event_Type event)
	    : m_event(event) {
	}

	bool match(Event_Type const &other) const {
		return other == m_event;
	}

	std::string describe() const override {
		std::stringstream stream;
		stream << "\nEquals:\n"
		       << m_event << "\n\nAll the events received:";
		for (auto const &event : Event_Handler::events())
		{
			stream << "\n" << event;
		}
		return stream.str();
	}

 private:
	Event_Type m_event;
};

template<typename T>
auto EqualsEvent(dsa::Allocation_Event_Type type, T *pointer, size_t count)
    -> EqualsMemoryMonitorEventMatcher {
	return EqualsMemoryMonitorEventMatcher(
	    Event_Type(dsa::Allocation_Event(type, pointer, count)));
}

template<typename T>
auto EqualsEvent(dsa::Object_Event_Type type, T *destination)
    -> EqualsMemoryMonitorEventMatcher {
	return EqualsMemoryMonitorEventMatcher(
	    Event_Type(dsa::Object_Event(type, destination)));
}

template<typename T>
auto EqualsEvent(dsa::Object_Event_Type type, T *destination, T const *source)
    -> EqualsMemoryMonitorEventMatcher {
	return EqualsMemoryMonitorEventMatcher(
	    Event_Type(dsa::Object_Event(type, destination, source)));
}

} // namespace test

#endif
