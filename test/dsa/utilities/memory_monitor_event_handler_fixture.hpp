#ifndef TEST_DSA_STATIC_MEMORY_MONITOR_EVENT_HANDLER_FIXTURE_HPP
#define TEST_DSA_STATIC_MEMORY_MONITOR_EVENT_HANDLER_FIXTURE_HPP

#include "memory_monitor_event_handler.hpp"

#include <catch2/catch_test_macros.hpp>

namespace test
{

class Memory_Monitor_Event_Handler_Fixture
{
 public:
	Memory_Monitor_Event_Handler_Fixture() {
		if (Event_Handler::instance() != nullptr)
		{
			std::cerr << "The handler's lifetime should "
				     "only be controlled by this object";
			std::terminate();
		}

		Event_Handler::instance() = std::make_unique<Event_Handler>();
	}

	~Memory_Monitor_Event_Handler_Fixture() {
		if (Event_Handler::instance() == nullptr)
		{
			std::cerr << "The handler's lifetime should only be "
				     "controlled by this object";
			std::terminate();
		}

		REQUIRE_NOTHROW(Event_Handler::instance()->cleanup());
		Event_Handler::instance() = nullptr;
	}

	Memory_Monitor_Event_Handler_Fixture(
	    Memory_Monitor_Event_Handler_Fixture const &) = delete;

	Memory_Monitor_Event_Handler_Fixture &operator=(
	    Memory_Monitor_Event_Handler_Fixture const &) = delete;

	Memory_Monitor_Event_Handler_Fixture(
	    Memory_Monitor_Event_Handler_Fixture &&) = delete;

	Memory_Monitor_Event_Handler_Fixture &operator=(
	    Memory_Monitor_Event_Handler_Fixture &&) = delete;
};

} // namespace test

#endif
