#include "empty_value.hpp"
#include "matchers/memory_monitor_event_matcher.hpp"
#include "utilities/memory_monitor_event_handler.hpp"
#include "utilities/memory_monitor_event_handler_fixture.hpp"

#include <dsa/memory_monitor.hpp>

#include <variant>
#include <vector>

#include <catch2/catch_all.hpp>

using namespace dsa;

namespace test
{

using Value = Element_Monitor<Empty_Value, Event_Handler>;

TEST_CASE_METHOD(
    Memory_Monitor_Event_Handler_Fixture,
    "Element Monitor correctly constructs objects with passed parameters",
    "[element_monitor]") {
	using Type = Element_Monitor<No_Default_Constructor_Value, Event_Handler>;

	Type value{No_Default_Constructor_Value_Construct_Tag()};
	REQUIRE_THAT(
	    Event_Handler::events().back(),
	    EqualsEvent(Object_Event_Type::Construct, &value.base()));
}

TEST_CASE_METHOD(
    Memory_Monitor_Event_Handler_Fixture,
    "Element monitor detects object construction and destruction",
    "[element_monitor]") {
	Empty_Value *address = nullptr;
	SECTION("Detect default construction") {
		Value value;
		address = &value.base();

		REQUIRE_THAT(
		    Event_Handler::events().back(),
		    EqualsEvent(Object_Event_Type::Construct, address));
	}

	SECTION("Detect construction from underlying type") {
		Value value(Empty_Value{});
		address = &value.base();

		REQUIRE_THAT(
		    Event_Handler::events().back(),
		    EqualsEvent(Object_Event_Type::Construct, address));
	}

	REQUIRE_THAT(Event_Handler::events().back(), EqualsEvent(Object_Event_Type::Destroy, address));
}

TEST_CASE_METHOD(
    Memory_Monitor_Event_Handler_Fixture,
    "Element monitor detects object copying",
    "[element_monitor]") {
	Value value;

	SECTION("Detect copy construction") {
		Value copy(value);

		REQUIRE_THAT(
		    Event_Handler::events().back(),
		    EqualsEvent(Object_Event_Type::Copy_Construct, &copy.base(), &value.base()));
	}

	SECTION("Detect copy assignment") {
		Value copy;
		copy = value;

		REQUIRE_THAT(
		    Event_Handler::events().back(),
		    EqualsEvent(Object_Event_Type::Copy_Assign, &copy.base(), &value.base()));
	}

	SECTION("Detect underlying copy assignment") {
		Empty_Value underlying;
		Value       copy;
		copy = underlying;

		REQUIRE_THAT(
		    Event_Handler::events().back(),
		    EqualsEvent(Object_Event_Type::Underlying_Copy_Assign, &copy.base()));
	}
}

TEST_CASE_METHOD(
    Memory_Monitor_Event_Handler_Fixture,
    "Element monitor detects object moving",
    "[element_monitor]") {
	Value temporary;

	SECTION("Detect move construction") {
		Value value(std::move(temporary));

		REQUIRE_THAT(
		    Event_Handler::events().back(),
		    EqualsEvent(Object_Event_Type::Move_Construct, &value.base(), &temporary.base()));
	}

	SECTION("Detect move assignment") {
		Value value;
		value = std::move(temporary);

		REQUIRE_THAT(
		    Event_Handler::events().back(),
		    EqualsEvent(Object_Event_Type::Move_Assign, &value.base(), &temporary.base()));
	}

	SECTION("Detect underlying move assignment") {
		Value value;
		value = Empty_Value{};

		REQUIRE_THAT(
		    Event_Handler::events().back(),
		    EqualsEvent(Object_Event_Type::Underlying_Move_Assign, &value.base()));
	}
}

} // namespace test
