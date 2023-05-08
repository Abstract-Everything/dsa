#include "empty_value.hpp"
#include "matchers/memory_monitor_event_matcher.hpp"
#include "utilities/memory_monitor_event_handler.hpp"
#include "utilities/memory_monitor_event_handler_fixture.hpp"

#include <dsa/memory_monitor.hpp>

#include <catch2/catch_all.hpp>

using namespace dsa;

namespace test
{

// clang-format off
using Handler_Scope = Memory_Monitor_Handler_Scope<Event_Handler>;
using Value         = Element_Monitor<Empty_Value, Event_Handler>;
using Pointer       = Element_Monitor_Pointer<false, Empty_Value, Event_Handler>;
using Const_Pointer = Element_Monitor_Pointer<true, Empty_Value, Event_Handler>;

// clang-format on

TEST_CASE_METHOD(
    Memory_Monitor_Event_Handler_Fixture,
    "Element monitor pointer detects construction and destruction",
    "[element_monitor_pointer]") {
	Value         value(Empty_Value{});
	Empty_Value **address = nullptr;
	SECTION("Detect default construction") {
		Pointer pointer;
		address = &pointer.base();

		REQUIRE(pointer == nullptr);

		REQUIRE_THAT(
		    Event_Handler::events().back(),
		    EqualsEvent(Object_Event_Type::Construct, address));
	}

	SECTION("Detect construction from underlying type") {
		Pointer pointer(&value);
		address = &pointer.base();

		REQUIRE_THAT(
		    Event_Handler::events().back(),
		    EqualsEvent(Object_Event_Type::Construct, address));
	}

	REQUIRE_THAT(
	    Event_Handler::events().back(),
	    EqualsEvent(Object_Event_Type::Destroy, address));
}

TEST_CASE_METHOD(
    Memory_Monitor_Event_Handler_Fixture,
    "Element monitor pointer has const-correct construction",
    "[element_monitor_pointer]") {
	SECTION("Const pointer can be constructed from non-const pointer") {
		STATIC_REQUIRE(std::is_constructible_v<Const_Pointer, Pointer>);
	}

	SECTION("Non-const pointer cannot be constructed from const pointer") {
		STATIC_REQUIRE(!std::is_constructible_v<Pointer, Const_Pointer>);
	}
}

TEST_CASE_METHOD(
    Memory_Monitor_Event_Handler_Fixture,
    "Element monitor pointer detects object copying",
    "[element_monitor_pointer]") {
	Value   value(Empty_Value{});
	Pointer pointer = &value;

	SECTION("Detect copy construction") {
		Pointer copy(pointer);

		REQUIRE_THAT(
		    Event_Handler::events().back(),
		    EqualsEvent(
			Object_Event_Type::Copy_Construct,
			&copy.base(),
			&pointer.base()));
	}

	SECTION("Detect copy assignment") {
		Pointer copy;
		copy = pointer;

		REQUIRE_THAT(
		    Event_Handler::events().back(),
		    EqualsEvent(
			Object_Event_Type::Copy_Assign,
			&copy.base(),
			&pointer.base()));
	}

	SECTION("Detect assignment to underlying value") {
		Pointer copy;
		copy = &value;

		REQUIRE_THAT(
		    Event_Handler::events().back(),
		    EqualsEvent(
			Object_Event_Type::Underlying_Copy_Assign,
			&copy.base()));
	}

	SECTION("Detect assignment to nullptr") {
		Pointer copy;
		copy = nullptr;

		REQUIRE_THAT(
		    Event_Handler::events().back(),
		    EqualsEvent(
			Object_Event_Type::Underlying_Copy_Assign,
			&copy.base()));
	}

	SECTION("Const pointer can be copied from non-const pointer") {
		STATIC_REQUIRE(std::is_constructible_v<Const_Pointer, Pointer>);
		STATIC_REQUIRE(std::is_assignable_v<Const_Pointer, Pointer>);
	}

	SECTION("Non-const pointer cannot be copied from const pointer") {
		STATIC_REQUIRE(!std::is_constructible_v<Pointer, Const_Pointer>);
		STATIC_REQUIRE(!std::is_assignable_v<Pointer, Const_Pointer>);
	}
}

TEST_CASE_METHOD(
    Memory_Monitor_Event_Handler_Fixture,
    "Element monitor pointer detects object moving",
    "[element_monitor_pointer]") {
	Pointer temporary;

	SECTION("Detect move construction") {
		Pointer pointer(std::move(temporary));

		REQUIRE_THAT(
		    Event_Handler::events().back(),
		    EqualsEvent(
			Object_Event_Type::Move_Construct,
			&pointer.base(),
			&temporary.base()));
	}

	SECTION("Detect move assignment") {
		Pointer pointer;
		pointer = std::move(temporary);

		REQUIRE_THAT(
		    Event_Handler::events().back(),
		    EqualsEvent(
			Object_Event_Type::Move_Assign,
			&pointer.base(),
			&temporary.base()));
	}

	SECTION("Const pointer can be moved from non-const pointer") {
		STATIC_REQUIRE(std::is_constructible_v<Const_Pointer, Pointer &&>);
		STATIC_REQUIRE(std::is_assignable_v<Const_Pointer, Pointer &&>);
	}

	SECTION("Non-const pointer cannot be moved from const pointer") {
		STATIC_REQUIRE(
		    !std::is_constructible_v<Pointer, Const_Pointer &&>);
		STATIC_REQUIRE(!std::is_assignable_v<Pointer, Const_Pointer &&>);
	}
}

TEST_CASE_METHOD(
    Memory_Monitor_Event_Handler_Fixture,
    "Element monitor pointer detects changes through operators",
    "[element_monitor_pointer]") {
	Value   value;
	Pointer pointer = &value;

	SECTION("Detect changes from pre-increment operator++") {
		Pointer modified = ++pointer;

		REQUIRE(modified.get() == pointer.get());
		REQUIRE(modified.get() == &value + 1);

		REQUIRE_THAT(
		    Event_Handler::instance()->last_event<Object_Event<Empty_Value *>>(
			{Object_Event_Type::Underlying_Copy_Assign}),
		    EqualsEvent(
			Object_Event_Type::Underlying_Copy_Assign,
			&pointer.base()));
	}

	SECTION("Detect changes from post-increment operator++") {
		Pointer modified = pointer++;

		REQUIRE(modified.get() + 1 == pointer.get());
		REQUIRE(modified.get() == &value);

		REQUIRE_THAT(
		    Event_Handler::events().back(),
		    EqualsEvent(
			Object_Event_Type::Underlying_Copy_Assign,
			&pointer.base()));
	}

	SECTION("Detect changes from pre-increment operator--") {
		Pointer modified = --pointer;

		REQUIRE(modified.get() == pointer.get());
		REQUIRE(modified.get() == &value - 1);

		REQUIRE_THAT(
		    Event_Handler::instance()->last_event<Object_Event<Empty_Value *>>(
			{Object_Event_Type::Underlying_Copy_Assign}),
		    EqualsEvent(
			Object_Event_Type::Underlying_Copy_Assign,
			&pointer.base()));
	}

	SECTION("Detect changes from post-increment operator--") {
		Pointer modified = pointer--;

		REQUIRE(modified.get() - 1 == pointer.get());
		REQUIRE(modified.get() == &value);

		REQUIRE_THAT(
		    Event_Handler::events().back(),
		    EqualsEvent(
			Object_Event_Type::Underlying_Copy_Assign,
			&pointer.base()));
	}

	SECTION("Support operator+") {
		int const offset   = 1;
		Pointer   modified = pointer + offset;

		REQUIRE(pointer.get() == &value);
		REQUIRE(modified.get() == &value + offset);

		REQUIRE_THAT(
		    Event_Handler::events().back(),
		    EqualsEvent(Object_Event_Type::Construct, &modified.base()));
	}

	SECTION("Support operator-") {
		int const offset   = 1;
		Pointer   modified = pointer - offset;

		REQUIRE(pointer.get() == &value);
		REQUIRE(modified.get() == &value - offset);

		REQUIRE_THAT(
		    Event_Handler::events().back(),
		    EqualsEvent(Object_Event_Type::Construct, &modified.base()));
	}

	SECTION("Detect changes from operator+=") {
		int const offset = 1;
		pointer += offset;

		REQUIRE(pointer.get() == &value + offset);

		REQUIRE_THAT(
		    Event_Handler::events().back(),
		    EqualsEvent(
			Object_Event_Type::Underlying_Copy_Assign,
			&pointer.base()));
	}

	SECTION("Detect changes from operator-=") {
		int const offset = 1;
		pointer -= offset;

		REQUIRE(pointer.get() == &value - offset);

		REQUIRE_THAT(
		    Event_Handler::events().back(),
		    EqualsEvent(
			Object_Event_Type::Underlying_Copy_Assign,
			&pointer.base()));
	}

	SECTION("Supports getting the difference with operator-") {
		long const expected = 2;
		Pointer    modified = pointer + expected;

		long offset = modified - pointer;

		REQUIRE(offset == expected);
	}
}

TEST_CASE_METHOD(
    Memory_Monitor_Event_Handler_Fixture,
    "Element monitor pointer supports comparison operators",
    "[element_monitor_pointer]") {
	Value         value;
	int           offset        = 3;
	Pointer       pointer       = &value;
	Const_Pointer const_pointer = pointer;
	Pointer       larger        = pointer + offset;

	SECTION("Supports equality comparison") {
		REQUIRE(pointer == &value);
		REQUIRE(pointer == const_pointer);
		REQUIRE_FALSE(pointer == larger);
	}

	SECTION("Supports inequality comparison") {
		REQUIRE_FALSE(pointer != &value);
		REQUIRE_FALSE(pointer != const_pointer);
		REQUIRE(pointer != larger);
	}

	SECTION("Supports operator<") {
		REQUIRE_FALSE(pointer < &value);
		REQUIRE_FALSE(pointer < const_pointer);
		REQUIRE_FALSE(larger < pointer);
		REQUIRE(pointer < larger);
	}
}

TEST_CASE_METHOD(
    Memory_Monitor_Event_Handler_Fixture,
    "Element monitor pointer supports pointer operators",
    "[element_monitor_pointer]") {
	Value         value;
	Value        *raw_pointer       = &value;
	Value const  *const_raw_pointer = &value;
	Pointer       pointer           = &value;
	Const_Pointer const_pointer     = &value;

	SECTION("Support operator[]") {
		REQUIRE(&pointer[1] == &raw_pointer[1]);
		REQUIRE(&pointer[2] == &raw_pointer[2]);
	}

	SECTION("Support operator*") {
		REQUIRE(&(pointer.operator*()) == raw_pointer);
		REQUIRE(&(const_pointer.operator*()) == const_raw_pointer);
	}

	SECTION("Support operator->") {
		REQUIRE(pointer.operator->() == raw_pointer);
		REQUIRE(const_pointer.operator->() == const_raw_pointer);
	}

	SECTION("get obtains the raw pointer") {
		REQUIRE(pointer.get() == raw_pointer);
		REQUIRE(const_pointer.get() == const_raw_pointer);
	}
}

} // namespace test
