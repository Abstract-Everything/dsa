#include "empty_value.hpp"
#include "memory_monitor_handler_scope.hpp"
#include "no_default_constructor_value.hpp"

#include <dsa/allocator_traits.hpp>
#include <dsa/memory_monitor.hpp>

#include <variant>

#include <catch2/catch_all.hpp>

namespace test
{

/// Data class used to store the parameters passed to an allocate or deallocate
/// call.
class Allocation_Event
{
 public:
	enum class Type
	{
		Allocate,
		Deallocate
	};

	Allocation_Event(Type type, void *pointer, size_t count)
	    : m_type(type)
	    , m_pointer(pointer)
	    , m_count(count)
	{
	}

	auto operator==(Allocation_Event const &event) const -> bool = default;
	friend auto operator<<(std::ostream &stream, Allocation_Event const &event)
	    -> std::ostream &;

 private:
	Type   m_type;
	void  *m_pointer;
	size_t m_count;
};

auto operator<<(std::ostream &stream, Allocation_Event::Type type)
    -> std::ostream &
{
	switch (type)
	{
	case Allocation_Event::Type::Allocate:
		stream << "Allocation";
		break;

	case Allocation_Event::Type::Deallocate:
		stream << "Deallocation";
		break;
	};
	return stream;
}

auto operator<<(std::ostream &stream, Allocation_Event const &event)
    -> std::ostream &
{
	stream << event.m_type << " at address "
	       << reinterpret_cast<uintptr_t>(event.m_pointer) << " with count "
	       << event.m_count;
	return stream;
}

/// Data class used to store the parameters passed to object lifetime
/// manipulation functions. These involve construction, destruction and moves.
class Object_Event
{
 public:
	enum class Type
	{
		Construct,
		Copy_Construct,
		Copy_Assign,
		Move_Construct,
		Move_Assign,
		Destroy
	};

	Object_Event(Type type, void *destination, void const *source = nullptr)
	    : m_type(type)
	    , m_destination(destination)
	    , m_source(source)
	{
	}

	auto operator==(Object_Event const &event) const -> bool = default;
	friend auto operator<<(std::ostream &stream, Object_Event const &event)
	    -> std::ostream &;

 private:
	Type        m_type;
	void       *m_destination;
	void const *m_source;
};

auto operator<<(std::ostream &stream, Object_Event::Type type) -> std::ostream &
{
	switch (type)
	{
	case Object_Event::Type::Construct:
		stream << "Construction";
		break;

	case Object_Event::Type::Destroy:
		stream << "Destruction";
		break;

	case Object_Event::Type::Copy_Construct:
		stream << "Copy construction";
		break;

	case Object_Event::Type::Copy_Assign:
		stream << "Copy assignment";
		break;

	case Object_Event::Type::Move_Construct:
		stream << "Move construction";
		break;

	case Object_Event::Type::Move_Assign:
		stream << "Move assignment";
		break;
	};
	return stream;
}

auto operator<<(std::ostream &stream, Object_Event const &event) -> std::ostream &
{
	stream << event.m_type << " at address "
	       << reinterpret_cast<uintptr_t>(event.m_destination);

	if (event.m_source == nullptr)
	{
		return stream;
	}

	switch (event.m_type)
	{
	case Object_Event::Type::Construct:
	case Object_Event::Type::Destroy:
		break;

	case Object_Event::Type::Copy_Construct:
	case Object_Event::Type::Copy_Assign:
	case Object_Event::Type::Move_Construct:
	case Object_Event::Type::Move_Assign:
		stream << " from address "
		       << reinterpret_cast<uintptr_t>(event.m_source);
		break;
	};

	return stream;
}

using Events = std::variant<Allocation_Event, Object_Event>;

auto operator<<(std::ostream &stream, Events const &event) -> std::ostream &
{
	std::visit([&](auto const &type) { stream << type; }, event);
	return stream;
}

template<typename T>
auto create_allocate_event(T *pointer, size_t count) -> Events
{
	return Allocation_Event(
	    Allocation_Event::Type::Allocate,
	    static_cast<void *>(pointer),
	    count);
}

template<typename T>
auto create_deallocate_event(T *pointer, size_t count) -> Events
{
	return Allocation_Event(
	    Allocation_Event::Type::Deallocate,
	    static_cast<void *>(pointer),
	    count);
}

template<typename T>
auto create_construct_event(T *destination) -> Events
{
	return Object_Event(
	    Object_Event::Type::Construct,
	    static_cast<void *>(destination));
}

template<typename T>
auto create_copy_construct_event(T *destination, T const *source) -> Events
{
	return Object_Event(
	    Object_Event::Type::Copy_Construct,
	    static_cast<void *>(destination),
	    static_cast<void const *>(source));
}

template<typename T>
auto create_copy_assign_event(T *destination, T const *source) -> Events
{
	return Object_Event(
	    Object_Event::Type::Copy_Assign,
	    static_cast<void *>(destination),
	    static_cast<void const *>(source));
}

template<typename T>
auto create_move_construct_event(T *destination, T const *source) -> Events
{
	return Object_Event(
	    Object_Event::Type::Move_Construct,
	    static_cast<void *>(destination),
	    static_cast<void const *>(source));
}

template<typename T>
auto create_move_assign_event(T *destination, T const *source) -> Events
{
	return Object_Event(
	    Object_Event::Type::Move_Assign,
	    static_cast<void *>(destination),
	    static_cast<void const *>(source));
}

template<typename T>
auto create_destroy_event(T *destination) -> Events
{
	return Object_Event(
	    Object_Event::Type::Destroy,
	    static_cast<void *>(destination));
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
	static void on_allocate(T *address, size_t count)
	{
		instance()->m_events.emplace_back(
		    create_allocate_event(address, count));
	}

	template<typename T>
	static void on_construct(T *address)
	{
		instance()->m_events.emplace_back(create_construct_event(address));
	}

	template<typename T>
	static void on_copy_construct(T *destination, T const *source)
	{
		instance()->m_events.emplace_back(
		    create_copy_construct_event(destination, source));
	}

	template<typename T>
	static void on_copy_assign(T *destination, T const *source)
	{
		instance()->m_events.emplace_back(
		    create_copy_assign_event(destination, source));
	}

	template<typename T>
	static void on_underlying_value_copy_assign(T *destination)
	{
		instance()->m_events.emplace_back(
		    create_copy_assign_event<T>(destination, nullptr));
	}

	template<typename T>
	static void on_move_construct(T *destination, T const *source)
	{
		instance()->m_events.emplace_back(
		    create_move_construct_event(destination, source));
	}

	template<typename T>
	static void on_move_assign(T *destination, T const *source)
	{
		instance()->m_events.emplace_back(
		    create_move_assign_event(destination, source));
	}

	template<typename T>
	static void on_underlying_value_move_assign(T *destination)
	{
		instance()->m_events.emplace_back(
		    create_move_assign_event<T>(destination, nullptr));
	}

	template<typename T>
	static void on_destroy(T *address)
	{
		instance()->m_events.emplace_back(create_destroy_event(address));
	}

	template<typename T>
	static auto before_deallocate(T * /* address */, size_t /* count */)
	    -> bool
	{
		return instance()->m_allow_deallocate;
	}

	template<typename T>
	static void on_deallocate(T *address, size_t count)
	{
		instance()->m_events.emplace_back(
		    create_deallocate_event(address, count));
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

	[[nodiscard]] auto events() -> std::vector<Events> const &
	{
		return m_events;
	}

 private:
	std::vector<Events> m_events;
	bool                m_allow_deallocate = true;
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
		REQUIRE(
		    Event_Handler::instance()->events().back()
		    == create_allocate_event(allocation, count));

		Alloc_Traits::deallocate(monitor, allocation, count);
	}

	SECTION("Monitor detects calls to deallocate")
	{
		Alloc_Traits::deallocate(monitor, allocation, count);

		REQUIRE(
		    Event_Handler::instance()->events().back()
		    == create_deallocate_event(allocation, count));
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

	REQUIRE(
	    Event_Handler::instance()->events().front()
	    == create_construct_event(address));

	REQUIRE(
	    Event_Handler::instance()->events().back()
	    == create_destroy_event(address));
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

		REQUIRE(
		    Event_Handler::instance()->events().back()
		    == create_construct_event(allocation));
	}

	SECTION("Monitor detects an uninitialised construct")
	{
		std::uninitialized_default_construct_n(allocation, 1);

		REQUIRE(
		    Event_Handler::instance()->events().back()
		    == create_construct_event(allocation));
	}

	SECTION("Monitor detects copy construction")
	{
		Allocator::Value value;
		std::uninitialized_fill_n(allocation, 1, value);

		REQUIRE(
		    Event_Handler::instance()->events().back()
		    == create_copy_construct_event(allocation, &value));
	}

	SECTION("An uninitialized_move sends a notification to the handler")
	{
		Allocator::Value value;
		std::uninitialized_move_n(&value, 1, allocation);

		REQUIRE(
		    Event_Handler::instance()->events().back()
		    == create_move_construct_event(allocation, &value));
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

	REQUIRE(
	    Event_Handler::instance()->events().back()
	    == create_construct_event(allocation));

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

	REQUIRE(
	    Event_Handler::instance()->events().back()
	    == create_construct_event(allocation));

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

		REQUIRE(
		    Event_Handler::instance()->events().back()
		    == create_copy_assign_event(allocation, &value));
	}

	SECTION("Monitor detects copy assignment from the underlying value")
	{
		*allocation = underlying_value;

		REQUIRE(
		    Event_Handler::instance()->events().back()
		    == create_copy_assign_event<Allocator::Value>(
			allocation,
			nullptr));
	}

	SECTION("Monitor detects move assignment")
	{
		*allocation = std::move(value);

		REQUIRE(
		    Event_Handler::instance()->events().back()
		    == create_move_assign_event(allocation, &value));
	}

	SECTION("Monitor detects move assignment from the underlying value")
	{
		*allocation = std::move(underlying_value);
		REQUIRE(
		    Event_Handler::instance()->events().back()
		    == create_move_assign_event<Allocator::Value>(
			allocation,
			nullptr));
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
		REQUIRE(
		    Event_Handler::instance()->events().back()
		    == create_destroy_event(allocation));
	}

	SECTION("Monitor detects destruction through destroy call")
	{
		std::destroy_n(allocation, 1);
		REQUIRE(
		    Event_Handler::instance()->events().back()
		    == create_destroy_event(allocation));
	}

	Alloc_Traits::deallocate(monitor, allocation, count);
}

} // namespace test
