#include "empty_value.hpp"
#include "matchers/memory_monitor_event_matcher.hpp"
#include "memory_monitor_handler_scope.hpp"
#include "no_default_constructor_value.hpp"
#include "utilities/memory_monitor_event_handler.hpp"

#include <dsa/allocator_traits.hpp>
#include <dsa/memory_monitor.hpp>

#include <any>
#include <variant>

#include <catch2/catch_all.hpp>

using namespace dsa;

namespace test
{

using Handler_Scope = Memory_Monitor_Handler_Scope<Event_Handler>;

TEST_CASE("No event is received if no actions were performed", "[monitor]")
{
	Handler_Scope scope;

	REQUIRE(Event_Handler::instance()->events().empty());
}

TEST_CASE("Monitor notifies handler about allocations", "[monitor]")
{
	using Allocator    = dsa::Memory_Monitor<Empty_Value, Event_Handler>;
	using Alloc_Traits = dsa::Allocator_Traits<Allocator>;

	Handler_Scope scope;

	Allocator    monitor;
	const size_t count      = 5;
	auto        *allocation = Alloc_Traits::allocate(monitor, count);

	SECTION("Monitor detects calls to allocate")
	{
		REQUIRE(Event_Handler::instance()->events().size() == 1);
		REQUIRE_THAT(
		    Event_Handler::instance()->events().back(),
		    EqualsEvent(
			Allocation_Event_Type::Allocate,
			&allocation->base(),
			count));

		Alloc_Traits::deallocate(monitor, allocation, count);
	}

	SECTION("Monitor detects calls to deallocate")
	{
		Alloc_Traits::deallocate(monitor, allocation, count);

		REQUIRE_THAT(
		    Event_Handler::instance()->events().back(),
		    EqualsEvent(
			Allocation_Event_Type::Deallocate,
			&allocation->base(),
			count));
	}

	SECTION("The handler can signal the Monitor to block a deallocate")
	{
		Event_Handler::instance()->block_deallocate();
		Alloc_Traits::deallocate(monitor, allocation, count);

		REQUIRE(Event_Handler::instance()->events().size() == 1);

		Event_Handler::instance()->unblock_deallocate();
		Alloc_Traits::deallocate(monitor, allocation, count);
	}
}

TEST_CASE(
    "Monitor detects construction and destruction of stack values",
    "[monitor]")
{
	using Allocator = dsa::Memory_Monitor<Empty_Value, Event_Handler>;
	Handler_Scope scope;

	Allocator::Value *address = nullptr;
	// Scope serves to call Value destructor
	{
		Allocator::Value value;
		address = &value;
	}

	REQUIRE_THAT(
	    Event_Handler::instance()->events().front(),
	    EqualsEvent(Object_Event_Type::Before_Construct, &address->base()));

	REQUIRE_THAT(
	    Event_Handler::instance()->events()[1],
	    EqualsEvent(Object_Event_Type::Construct, &address->base()));

	REQUIRE_THAT(
	    Event_Handler::instance()->events().back(),
	    EqualsEvent(Object_Event_Type::Destroy, &address->base()));
}

TEST_CASE("Monitor notifies handler about object construction", "[monitor]")
{
	using Allocator    = dsa::Memory_Monitor<Empty_Value, Event_Handler>;
	using Alloc_Traits = dsa::Allocator_Traits<Allocator>;

	Handler_Scope scope;
	Allocator     monitor;

	size_t count      = 1;
	auto  *allocation = Alloc_Traits::allocate(monitor, count);

	SECTION("Monitor detects construction through allocation traits")
	{
		Alloc_Traits::construct(
		    monitor,
		    allocation,
		    Allocator::Underlying_Value{});

		REQUIRE_THAT(
		    Event_Handler::instance()->events().back(),
		    EqualsEvent(Object_Event_Type::Construct, &allocation->base()));
	}

	SECTION("Monitor detects an uninitialised construct")
	{
		std::uninitialized_default_construct_n(allocation, 1);

		REQUIRE_THAT(
		    Event_Handler::instance()->events().back(),
		    EqualsEvent(Object_Event_Type::Construct, &allocation->base()));
	}

	SECTION("Monitor detects copy construction")
	{
		Allocator::Value value;
		std::uninitialized_fill_n(allocation, 1, value);

		REQUIRE_THAT(
		    Event_Handler::instance()->events().back(),
		    EqualsEvent(
			Object_Event_Type::Copy_Construct,
			&allocation->base(),
			&value.base()));
	}

	SECTION("An uninitialized_move sends a notification to the handler")
	{
		Allocator::Value value;
		std::uninitialized_move_n(&value, 1, allocation);

		REQUIRE_THAT(
		    Event_Handler::instance()->events().back(),
		    EqualsEvent(
			Object_Event_Type::Move_Construct,
			&allocation->base(),
			&value.base()));
	}

	Alloc_Traits::destroy(monitor, allocation);
	Alloc_Traits::deallocate(monitor, allocation, count);
}

TEST_CASE(
    "Monitor correctly constructs objects with passed parameters",
    "[monitor]")
{
	using Allocator =
	    dsa::Memory_Monitor<No_Default_Constructor_Value, Event_Handler>;
	using Alloc_Traits = dsa::Allocator_Traits<Allocator>;

	Handler_Scope scope;

	Allocator monitor;

	size_t count      = 1;
	auto  *allocation = Alloc_Traits::allocate(monitor, count);
	Alloc_Traits::construct(
	    monitor,
	    allocation,
	    No_Default_Constructor_Value_Construct_Tag());

	REQUIRE_THAT(
	    Event_Handler::instance()->events().back(),
	    EqualsEvent(Object_Event_Type::Construct, &allocation->base()));

	Alloc_Traits::destroy(monitor, allocation);
	Alloc_Traits::deallocate(monitor, allocation, count);
}

TEST_CASE(
    "Element_Monitor can be constructed from the underlying value",
    "[monitor]")
{
	using Allocator    = dsa::Memory_Monitor<Empty_Value, Event_Handler>;
	using Alloc_Traits = dsa::Allocator_Traits<Allocator>;

	Handler_Scope scope;

	Allocator monitor;

	size_t count      = 1;
	auto  *allocation = Alloc_Traits::allocate(monitor, count);
	Alloc_Traits::construct(monitor, allocation, Allocator::Underlying_Value());

	REQUIRE_THAT(
	    Event_Handler::instance()->events().back(),
	    EqualsEvent(Object_Event_Type::Construct, &allocation->base()));

	Alloc_Traits::destroy(monitor, allocation);
	Alloc_Traits::deallocate(monitor, allocation, count);
}

TEST_CASE("Monitor notifies handler about object assignment", "[monitor]")
{
	using Allocator    = dsa::Memory_Monitor<Empty_Value, Event_Handler>;
	using Alloc_Traits = dsa::Allocator_Traits<Allocator>;

	Handler_Scope scope;

	Allocator        monitor;
	Allocator::Value value;
	Empty_Value      underlying_value;

	size_t count      = 1;
	auto  *allocation = Alloc_Traits::allocate(monitor, count);
	std::uninitialized_default_construct_n(allocation, 1);

	SECTION("Monitor detects copy assignment")
	{
		*allocation = value;

		REQUIRE_THAT(
		    Event_Handler::instance()->events().back(),
		    EqualsEvent(
			Object_Event_Type::Copy_Assign,
			&allocation->base(),
			&value.base()));
	}

	SECTION("Monitor detects copy assignment from the underlying value")
	{
		*allocation = underlying_value;

		REQUIRE_THAT(
		    Event_Handler::instance()->events().back(),
		    EqualsEvent(
			Object_Event_Type::Underlying_Copy_Assign,
			&allocation->base()));
	}

	SECTION("Monitor detects move assignment")
	{
		*allocation = std::move(value);

		REQUIRE_THAT(
		    Event_Handler::instance()->events().back(),
		    EqualsEvent(
			Object_Event_Type::Move_Assign,
			&allocation->base(),
			&value.base()));
	}

	SECTION("Monitor detects move assignment from the underlying value")
	{
		*allocation = std::move(underlying_value);
		REQUIRE_THAT(
		    Event_Handler::instance()->events().back(),
		    EqualsEvent(
			Object_Event_Type::Underlying_Move_Assign,
			&allocation->base()));
	}

	Alloc_Traits::destroy(monitor, allocation);
	Alloc_Traits::deallocate(monitor, allocation, count);
}

TEST_CASE("Monitor notifies handler about object destruction", "[monitor]")
{
	using Allocator    = dsa::Memory_Monitor<Empty_Value, Event_Handler>;
	using Alloc_Traits = dsa::Allocator_Traits<Allocator>;

	Handler_Scope scope;

	Allocator monitor;

	size_t count      = 1;
	auto  *allocation = Alloc_Traits::allocate(monitor, count);
	Alloc_Traits::construct(monitor, allocation);

	SECTION("Monitor detects destruction through allocation traits")
	{
		Alloc_Traits::destroy(monitor, allocation);
		REQUIRE_THAT(
		    Event_Handler::instance()->events().back(),
		    EqualsEvent(Object_Event_Type::Destroy, &allocation->base()));
	}

	SECTION("Monitor detects destruction through destroy call")
	{
		std::destroy_n(allocation, 1);
		REQUIRE_THAT(
		    Event_Handler::instance()->events().back(),
		    EqualsEvent(Object_Event_Type::Destroy, &allocation->base()));
	}

	Alloc_Traits::deallocate(monitor, allocation, count);
}

} // namespace test
