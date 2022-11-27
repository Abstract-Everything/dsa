#include "empty_value.hpp"
#include "memory_monitor_handler_scope.hpp"
#include "no_default_constructor_value.hpp"

#include <dsa/allocator_traits.hpp>
#include <dsa/memory_monitor.hpp>

#include <any>
#include <variant>

#include <catch2/catch_all.hpp>

using namespace dsa;

namespace test
{

template<typename T>
using Event_Type = std::variant<Allocation_Event<T>, Object_Event<T>>;

class Memory_Monitor_Event
{
 public:
	virtual ~Memory_Monitor_Event() = default;

	friend auto operator<<(std::ostream &stream, Memory_Monitor_Event const &event)
	    -> std::ostream &
	{
		event.stream_impl(stream);
		return stream;
	}

	[[nodiscard]] virtual auto equals(std::any value) const -> bool = 0;

 protected:
	Memory_Monitor_Event() = default;

	virtual void stream_impl(std::ostream &stream) const = 0;
};

template<typename T>
class Memory_Monitor_Event_Typed : public Memory_Monitor_Event
{
 public:
	explicit Memory_Monitor_Event_Typed(Allocation_Event<T> event)
	    : m_event(event)
	{
	}

	explicit Memory_Monitor_Event_Typed(Object_Event<T> event)
	    : m_event(event)
	{
	}

	void stream_impl(std::ostream &stream) const override
	{
		std::visit([&](auto const &event) { stream << event; }, m_event);
	}

	[[nodiscard]] auto equals(std::any value) const -> bool override
	{
		try
		{
			return std::any_cast<Event_Type<T>>(value) == m_event;
		}
		catch (...)
		{
			return false;
		}
	}

 private:
	Event_Type<T> m_event;
};

template<typename T>
struct EqualsMemoryMonitorEventMatcher : Catch::Matchers::MatcherGenericBase
{
	EqualsMemoryMonitorEventMatcher(Event_Type<T> event) : m_event(event)
	{
	}

	bool match(std::unique_ptr<Memory_Monitor_Event> const &other) const
	{
		return other->equals(m_event);
	}

	std::string describe() const override
	{
		std::stringstream stream;
		std::visit([&](auto const &event) { stream << event; }, m_event);
		return "Equals: " + stream.str();
	}

 private:
	Event_Type<T> m_event;
};

template<typename T>
auto EqualsEvent(Allocation_Event_Type type, T *pointer, size_t count)
    -> EqualsMemoryMonitorEventMatcher<T>
{
	return EqualsMemoryMonitorEventMatcher(
	    Event_Type<T>(Allocation_Event(type, pointer, count)));
}

template<typename T>
auto EqualsEvent(
    Object_Event_Type type,
    T                *destination,
    T const          *source = nullptr) -> EqualsMemoryMonitorEventMatcher<T>
{
	return EqualsMemoryMonitorEventMatcher(
	    Event_Type<T>(Object_Event(type, destination, source)));
}

/// Maintains a list events produced by the appropriate callback. The entries
/// can be analysed to determine if the Memory_Monitor is calling the
/// appropriate callbacks in the appropriate order
class Event_Handler
{
 public:
	static auto instance() -> std::unique_ptr<Event_Handler> &
	{
		static std::unique_ptr<Event_Handler> instance = nullptr;
		return instance;
	}

	template<typename T>
	static auto before_deallocate(Allocation_Event<T> /* event */) -> bool
	{
		return instance()->m_allow_deallocate;
	}

	template<typename T>
	static void process_allocation_event(Allocation_Event<T> event)
	{
		instance()->m_events.push_back(
		    std::make_unique<Memory_Monitor_Event_Typed<T>>(event));
	}

	template<typename T>
	static void process_object_event(Object_Event<T> event)
	{
		instance()->m_events.push_back(
		    std::make_unique<Memory_Monitor_Event_Typed<T>>(event));
	}

	void cleanup()
	{
	}

	void block_deallocate()
	{
		m_allow_deallocate = false;
	}

	void unblock_deallocate()
	{
		m_allow_deallocate = true;
	}

	[[nodiscard]] auto events()
	    -> std::vector<std::unique_ptr<Memory_Monitor_Event>> const &
	{
		return m_events;
	}

 private:
	std::vector<std::unique_ptr<Memory_Monitor_Event>> m_events;
	bool m_allow_deallocate = true;
};

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
		    EqualsEvent(Allocation_Event_Type::Allocate, allocation, count));

		Alloc_Traits::deallocate(monitor, allocation, count);
	}

	SECTION("Monitor detects calls to deallocate")
	{
		Alloc_Traits::deallocate(monitor, allocation, count);

		REQUIRE_THAT(
		    Event_Handler::instance()->events().back(),
		    EqualsEvent(Allocation_Event_Type::Deallocate, allocation, count));
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
	    EqualsEvent(Object_Event_Type::Construct, address));

	REQUIRE_THAT(
	    Event_Handler::instance()->events().back(),
	    EqualsEvent(Object_Event_Type::Destroy, address));
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
		    EqualsEvent(Object_Event_Type::Construct, allocation));
	}

	SECTION("Monitor detects an uninitialised construct")
	{
		std::uninitialized_default_construct_n(allocation, 1);

		REQUIRE_THAT(
		    Event_Handler::instance()->events().back(),
		    EqualsEvent(Object_Event_Type::Construct, allocation));
	}

	SECTION("Monitor detects copy construction")
	{
		Allocator::Value value;
		std::uninitialized_fill_n(allocation, 1, value);

		REQUIRE_THAT(
		    Event_Handler::instance()->events().back(),
		    EqualsEvent(Object_Event_Type::Copy_Construct, allocation, &value));
	}

	SECTION("An uninitialized_move sends a notification to the handler")
	{
		Allocator::Value value;
		std::uninitialized_move_n(&value, 1, allocation);

		REQUIRE_THAT(
		    Event_Handler::instance()->events().back(),
		    EqualsEvent(Object_Event_Type::Move_Construct, allocation, &value));
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
	    EqualsEvent(Object_Event_Type::Construct, allocation));

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
	    EqualsEvent(Object_Event_Type::Construct, allocation));

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
		    EqualsEvent(Object_Event_Type::Copy_Assign, allocation, &value));
	}

	SECTION("Monitor detects copy assignment from the underlying value")
	{
		*allocation = underlying_value;

		REQUIRE_THAT(
		    Event_Handler::instance()->events().back(),
		    EqualsEvent(
			Object_Event_Type::Underlying_Copy_Assign,
			allocation));
	}

	SECTION("Monitor detects move assignment")
	{
		*allocation = std::move(value);

		REQUIRE_THAT(
		    Event_Handler::instance()->events().back(),
		    EqualsEvent(Object_Event_Type::Move_Assign, allocation, &value));
	}

	SECTION("Monitor detects move assignment from the underlying value")
	{
		*allocation = std::move(underlying_value);
		REQUIRE_THAT(
		    Event_Handler::instance()->events().back(),
		    EqualsEvent(
			Object_Event_Type::Underlying_Move_Assign,
			allocation));
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
		    EqualsEvent(Object_Event_Type::Destroy, allocation));
	}

	SECTION("Monitor detects destruction through destroy call")
	{
		std::destroy_n(allocation, 1);
		REQUIRE_THAT(
		    Event_Handler::instance()->events().back(),
		    EqualsEvent(Object_Event_Type::Destroy, allocation));
	}

	Alloc_Traits::deallocate(monitor, allocation, count);
}

} // namespace test
