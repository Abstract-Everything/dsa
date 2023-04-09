#include "allocation_verifier.hpp"
#include "empty_value.hpp"
#include "memory_monitor_handler_scope.hpp"

#include <dsa/allocator_traits.hpp>
#include <dsa/memory_monitor.hpp>

#include <catch2/catch_all.hpp>

namespace test
{

// Note that we do not use SECTION s because if the Allocation_Verifier calls
// std::terminate it is difficult to know which SECTION failed

using Single_Field_Allocator =
    dsa::Memory_Monitor<Empty_Value, Allocation_Verifier>;
using Single_Field_Alloc_Traits = dsa::Allocator_Traits<Single_Field_Allocator>;
using Handler_Scope = Memory_Monitor_Handler_Scope<Allocation_Verifier>;

TEST_CASE("Default construction reports no errors", "[allocation_verifier]")
{
	Handler_Scope scope;
	REQUIRE_NOTHROW(Allocation_Verifier::instance()->cleanup());
}

TEST_CASE("Monitor detects memory leak", "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	Single_Field_Alloc_Traits::allocate(verifier, 1);

	REQUIRE_THROWS_WITH(
	    Allocation_Verifier::instance()->cleanup(),
	    memory_leaked);
}

TEST_CASE(
    "Monitor verifies deallocate address matches previous allocate address",
    "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	size_t count  = 2;
	auto   memory = Single_Field_Alloc_Traits::allocate(verifier, count);

	Single_Field_Alloc_Traits::deallocate(verifier, memory + 1, count);

	REQUIRE_THROWS_WITH(
	    Allocation_Verifier::instance()->cleanup(),
	    Catch::Matchers::ContainsSubstring(deallocating_unallocated_memory));
}

TEST_CASE(
    "Monitor verifiers deallocate count matches previous allocate count",
    "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	size_t count  = 2;
	auto   memory = Single_Field_Alloc_Traits::allocate(verifier, count);

	Single_Field_Alloc_Traits::deallocate(verifier, memory, count + 1);

	REQUIRE_THROWS_WITH(
	    Allocation_Verifier::instance()->cleanup(),
	    Catch::Matchers::ContainsSubstring(deallocating_count_mismatch));
}

TEST_CASE(
    "An allocate followed by a valid deallocate raises no error",
    "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	size_t count  = 1;
	auto   memory = Single_Field_Alloc_Traits::allocate(verifier, count);

	Single_Field_Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_NOTHROW(Allocation_Verifier::instance()->cleanup());
}

TEST_CASE("Allocations can only be deallocated once", "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	size_t count    = 2;
	auto   memory_a = Single_Field_Alloc_Traits::allocate(verifier, count);
	auto   memory_b = Single_Field_Alloc_Traits::allocate(verifier, count);

	Single_Field_Alloc_Traits::deallocate(verifier, memory_a, count);
	Single_Field_Alloc_Traits::deallocate(verifier, memory_a, count);
	Single_Field_Alloc_Traits::deallocate(verifier, memory_b, count);

	REQUIRE_THROWS_WITH(
	    Allocation_Verifier::instance()->cleanup(),
	    deallocating_unallocated_memory);
}

TEST_CASE(
    "Multiple allocations each require a deallocation",
    "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	size_t count_a = 1;
	auto  memory_a = Single_Field_Alloc_Traits::allocate(verifier, count_a);
	size_t count_b = 2;
	auto  memory_b = Single_Field_Alloc_Traits::allocate(verifier, count_b);
	size_t count_c = 3;
	Single_Field_Alloc_Traits::allocate(verifier, count_c);

	Single_Field_Alloc_Traits::deallocate(verifier, memory_a, count_a);
	Single_Field_Alloc_Traits::deallocate(verifier, memory_b, count_b);

	REQUIRE_THROWS_WITH(
	    Allocation_Verifier::instance()->cleanup(),
	    memory_leaked);
}

TEST_CASE(
    "Deallocating constructed elements raises an error",
    "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	size_t count  = 1;
	auto   memory = Single_Field_Alloc_Traits::allocate(verifier, count);

	Single_Field_Alloc_Traits::construct(verifier, memory, Empty_Value());
	Single_Field_Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_THROWS_WITH(
	    Allocation_Verifier::instance()->cleanup(),
	    object_leaked);
}

TEST_CASE(
    "Constructed elements left undestroyed raise an error",
    "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	size_t count  = 1;
	auto   memory = Single_Field_Alloc_Traits::allocate(verifier, count);

	std::uninitialized_default_construct_n(memory, count);

	Single_Field_Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_THROWS_WITH(
	    Allocation_Verifier::instance()->cleanup(),
	    object_leaked);
}

TEST_CASE(
    "Calling construct on already constructed memory raises an error",
    "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	size_t count  = 1;
	auto   memory = Single_Field_Alloc_Traits::allocate(verifier, count);

	Single_Field_Alloc_Traits::construct(verifier, memory, Empty_Value());
	Single_Field_Alloc_Traits::construct(verifier, memory, Empty_Value());

	Single_Field_Alloc_Traits::destroy(verifier, memory);
	Single_Field_Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_THROWS_WITH(
	    Allocation_Verifier::instance()->cleanup(),
	    object_leaked);
}

TEST_CASE(
    "Objects initialised through a copy also require destruction",
    "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	size_t count  = 1;
	auto   memory = Single_Field_Alloc_Traits::allocate(verifier, count);

	Single_Field_Allocator::Value value;
	std::uninitialized_fill_n(memory, 1, value);

	Single_Field_Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_THROWS_WITH(
	    Allocation_Verifier::instance()->cleanup(),
	    object_leaked);
}

TEST_CASE(
    "Objects initialised through move construction also require destruction",
    "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	size_t count  = 1;
	auto   memory = Single_Field_Alloc_Traits::allocate(verifier, count);

	Single_Field_Allocator::Value value;
	std::uninitialized_move_n(&value, 1, memory);

	Single_Field_Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_THROWS_WITH(
	    Allocation_Verifier::instance()->cleanup(),
	    object_leaked);
}

TEST_CASE(
    "Copy construct from uninitialised memory raises an error",
    "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	size_t count    = 1;
	auto   memory_a = Single_Field_Alloc_Traits::allocate(verifier, count);
	auto   memory_b = Single_Field_Alloc_Traits::allocate(verifier, count);

	std::uninitialized_fill_n(memory_b, 1, *memory_a);

	Single_Field_Alloc_Traits::deallocate(verifier, memory_a, count);
	Single_Field_Alloc_Traits::deallocate(verifier, memory_b, count);

	REQUIRE_THROWS_WITH(
	    Allocation_Verifier::instance()->cleanup(),
	    Catch::Matchers::ContainsSubstring(assign_from_uninitialized_memory));
}

TEST_CASE(
    "Move construct from uninitialised memory raises an error",
    "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	size_t count    = 1;
	auto   memory_a = Single_Field_Alloc_Traits::allocate(verifier, count);
	auto   memory_b = Single_Field_Alloc_Traits::allocate(verifier, count);

	std::uninitialized_move_n(memory_a, 1, memory_b);

	Single_Field_Alloc_Traits::deallocate(verifier, memory_a, count);
	Single_Field_Alloc_Traits::deallocate(verifier, memory_b, count);

	REQUIRE_THROWS_WITH(
	    Allocation_Verifier::instance()->cleanup(),
	    Catch::Matchers::ContainsSubstring(assign_from_uninitialized_memory));
}

TEST_CASE(
    "Copy assign from uninitialised memory raises an error",
    "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	size_t count    = 1;
	auto   memory_a = Single_Field_Alloc_Traits::allocate(verifier, count);
	auto   memory_b = Single_Field_Alloc_Traits::allocate(verifier, count);
	Single_Field_Alloc_Traits::construct(verifier, memory_b);

	*memory_b = *memory_a;

	Single_Field_Alloc_Traits::deallocate(verifier, memory_a, count);
	Single_Field_Alloc_Traits::deallocate(verifier, memory_b, count);

	REQUIRE_THROWS_WITH(
	    Allocation_Verifier::instance()->cleanup(),
	    Catch::Matchers::ContainsSubstring(assign_from_uninitialized_memory));
}

TEST_CASE(
    "Move assign from uninitialised memory raises an error",
    "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	size_t count    = 1;
	auto   memory_a = Single_Field_Alloc_Traits::allocate(verifier, count);
	auto   memory_b = Single_Field_Alloc_Traits::allocate(verifier, count);
	Single_Field_Alloc_Traits::construct(verifier, memory_b);

	*memory_b = std::move(*memory_a);

	Single_Field_Alloc_Traits::deallocate(verifier, memory_a, count);
	Single_Field_Alloc_Traits::deallocate(verifier, memory_b, count);

	REQUIRE_THROWS_WITH(
	    Allocation_Verifier::instance()->cleanup(),
	    Catch::Matchers::ContainsSubstring(assign_from_uninitialized_memory));
}

TEST_CASE(
    "Assign to uninitialized memory raises an error",
    "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	size_t count  = 1;
	auto   memory = Single_Field_Alloc_Traits::allocate(verifier, count);

	Single_Field_Allocator::Value value;
	*memory = value;

	Single_Field_Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_THROWS_WITH(
	    Allocation_Verifier::instance()->cleanup(),
	    assign_uninitialized_memory);
}

TEST_CASE(
    "Performing move assign on uninitialized memory raises an error",
    "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	size_t count  = 1;
	auto   memory = Single_Field_Alloc_Traits::allocate(verifier, 1);

	Single_Field_Allocator::Value value;
	*memory = std::move(value);

	Single_Field_Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_THROWS_WITH(
	    Allocation_Verifier::instance()->cleanup(),
	    assign_uninitialized_memory);
}

TEST_CASE(
    "Values can be used to copy construct and assign multiple times",
    "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	size_t count  = 2;
	auto   memory = Single_Field_Alloc_Traits::allocate(verifier, count);

	Single_Field_Allocator::Value value;
	Single_Field_Alloc_Traits::construct(verifier, memory, value);
	*memory = value;
	Single_Field_Alloc_Traits::construct(verifier, memory + 1, value);
	*(memory + 1) = value;

	std::destroy_n(memory, count);
	Single_Field_Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_NOTHROW(Allocation_Verifier::instance()->cleanup());
}

TEST_CASE(
    "Values can be used to move construct only once",
    "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	size_t count  = 1;
	auto   memory = Single_Field_Alloc_Traits::allocate(verifier, count);

	Single_Field_Allocator::Value value;
	Single_Field_Alloc_Traits::construct(verifier, memory, std::move(value));
	Single_Field_Alloc_Traits::construct(verifier, memory, std::move(value));

	std::destroy_n(memory, count);
	Single_Field_Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_THROWS(
	    Allocation_Verifier::instance()->cleanup(),
	    assign_from_uninitialized_memory);
}

TEST_CASE("Values can be used to move assign only once", "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	size_t count  = 1;
	auto   memory = Single_Field_Alloc_Traits::allocate(verifier, count);

	Single_Field_Allocator::Value value;
	*memory = std::move(value);
	*memory = std::move(value);

	std::destroy_n(memory, count);
	Single_Field_Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_THROWS(
	    Allocation_Verifier::instance()->cleanup(),
	    assign_from_uninitialized_memory);
}

TEST_CASE("Construct can be called on moved values", "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	size_t count  = 1;
	auto   memory = Single_Field_Alloc_Traits::allocate(verifier, count);

	Single_Field_Alloc_Traits::construct(verifier, memory);
	Single_Field_Allocator::Value value(std::move(*memory));
	Single_Field_Alloc_Traits::construct(verifier, memory);

	std::destroy_n(memory, count);
	Single_Field_Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_NOTHROW(Allocation_Verifier::instance()->cleanup());
}

TEST_CASE("Moved memory can be reassigned", "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	size_t count  = 1;
	auto   memory = Single_Field_Alloc_Traits::allocate(verifier, count);

	Single_Field_Alloc_Traits::construct(verifier, memory);
	Single_Field_Allocator::Value value(std::move(*memory));
	*memory = std::move(value);

	std::destroy_n(memory, count);
	Single_Field_Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_NOTHROW(Allocation_Verifier::instance()->cleanup());
}

TEST_CASE("Moved into memory is considered initialised", "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	size_t count  = 1;
	auto   memory = Single_Field_Alloc_Traits::allocate(verifier, count);

	Single_Field_Alloc_Traits::construct(verifier, memory);
	Single_Field_Allocator::Value value(std::move(*memory));
	*memory = std::move(value);
	Single_Field_Allocator::Value value2(std::move(*memory));

	std::destroy_n(memory, count);
	Single_Field_Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_NOTHROW(Allocation_Verifier::instance()->cleanup());
}

TEST_CASE(
    "Calling destroy on unconstructed memory raises an error",
    "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	size_t count  = 1;
	auto   memory = Single_Field_Alloc_Traits::allocate(verifier, count);

	Single_Field_Alloc_Traits::destroy(verifier, memory);

	Single_Field_Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_THROWS_WITH(
	    Allocation_Verifier::instance()->cleanup(),
	    destroying_nonconstructed_memory);
}

TEST_CASE(
    "Objects move constructed from allocated memory require no destruction",
    "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	size_t count  = 1;
	auto   memory = Single_Field_Alloc_Traits::allocate(verifier, count);
	Single_Field_Alloc_Traits::construct(verifier, memory, Empty_Value());

	Single_Field_Allocator::Value value(std::move(*memory));

	Single_Field_Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_NOTHROW(Allocation_Verifier::instance()->cleanup());
}

TEST_CASE(
    "Objects move assigned from allocated memory require no destruction",
    "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	size_t count  = 1;
	auto   memory = Single_Field_Alloc_Traits::allocate(verifier, count);
	Single_Field_Alloc_Traits::construct(verifier, memory, Empty_Value());

	Single_Field_Allocator::Value value;
	value = std::move(*memory);

	Single_Field_Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_NOTHROW(Allocation_Verifier::instance()->cleanup());
}

TEST_CASE(
    "Objects copied from allocated memory still require desturction",
    "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	size_t count  = 1;
	auto   memory = Single_Field_Alloc_Traits::allocate(verifier, count);

	Single_Field_Alloc_Traits::construct(verifier, memory, Empty_Value());
	Single_Field_Allocator::Value value(*memory);

	Single_Field_Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_THROWS_WITH(
	    Allocation_Verifier::instance()->cleanup(),
	    object_leaked);
}

TEST_CASE(
    "Objects created through move assignment still require destruction",
    "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	size_t count  = 1;
	auto   memory = Single_Field_Alloc_Traits::allocate(verifier, 1);

	Single_Field_Alloc_Traits::construct(verifier, memory, Empty_Value());
	Single_Field_Allocator::Value value;
	*memory = std::move(value);

	Single_Field_Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_THROWS_WITH(
	    Allocation_Verifier::instance()->cleanup(),
	    object_leaked);
}

TEST_CASE(
    "A construct followed by a destroy raises no error",
    "[allocation_verifier]")
{
	Handler_Scope          scope;
	Single_Field_Allocator verifier;

	size_t count  = 2;
	auto   memory = Single_Field_Alloc_Traits::allocate(verifier, count);

	Single_Field_Alloc_Traits::construct(verifier, memory, Empty_Value());
	Single_Field_Alloc_Traits::construct(verifier, memory + 1, Empty_Value());

	Single_Field_Alloc_Traits::destroy(verifier, memory + 1);
	Single_Field_Alloc_Traits::destroy(verifier, memory);

	Single_Field_Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_NOTHROW(Allocation_Verifier::instance()->cleanup());
}

TEST_CASE(
    "Objects allocated on the stack do not raise errors",
    "[allocation_verifier]")
{
	Handler_Scope scope;
	{
		Single_Field_Allocator::Value value;
	}
	REQUIRE_NOTHROW(Allocation_Verifier::instance()->cleanup());
}

TEST_CASE("allocated on the stack do not raise errors", "[allocation_verifier]")
{
	Handler_Scope scope;
	{
		Single_Field_Allocator::Value value_a;
		Single_Field_Allocator::Value value_b(std::move(value_a));
	}
	REQUIRE_NOTHROW(Allocation_Verifier::instance()->cleanup());
}

struct Multiple_Field_Struct
{
	typename Single_Field_Allocator::Value field_a;
	typename Single_Field_Allocator::Value field_b;

	Multiple_Field_Struct() = default;

	friend auto operator<<(std::ostream &stream, Multiple_Field_Struct const &data)
	    -> std::ostream &
	{
		// clang-format off
		return stream << '{'
			          << data.field_a
			          << ',' << data.field_b
			      << '}';
		// clang-format on
	}
};

using Multiple_Field_Allocator =
    dsa::Memory_Monitor<Multiple_Field_Struct, Allocation_Verifier>;

using Multiple_Field_Alloc_Traits =
    dsa::Allocator_Traits<Multiple_Field_Allocator>;

TEST_CASE(
    "Complex structures can have overlapping construct calls",
    "[allocation_verifier]")
{
	Handler_Scope            scope;
	Multiple_Field_Allocator verifier;

	size_t count  = 1;
	auto   memory = Multiple_Field_Alloc_Traits::allocate(verifier, count);
	Multiple_Field_Alloc_Traits::construct(verifier, memory);
	Multiple_Field_Alloc_Traits::destroy(verifier, memory);
	Multiple_Field_Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_NOTHROW(Allocation_Verifier::instance()->cleanup());
}

} // namespace test
