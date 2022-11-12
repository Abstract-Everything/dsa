#include "allocation_verifier.hpp"
#include "empty_value.hpp"
#include "memory_monitor_scope.hpp"

#include <dsa/allocator_traits.hpp>
#include <dsa/memory_monitor.hpp>

#include <catch2/catch_all.hpp>

namespace test
{

// Note that we do not use SECTION s because if the Allocation_Verifier calls
// std::terminate it is difficult to know which SECTION failed

using Allocator    = dsa::Memory_Monitor<Empty_Value, Allocation_Verifier>;
using Alloc_Traits = dsa::Allocator_Traits<Allocator>;
using Allocator_Scope =
    Memory_Monitor_Scope<Allocator::Underlying_Value, Allocation_Verifier>;

TEST_CASE("Default construction reports no errors", "[allocation_verifier]")
{
	Allocator_Scope scope;
	Allocator       verifier;
	REQUIRE_NOTHROW(Allocator::handler()->cleanup());
}

TEST_CASE("Monitor detects memory leak", "[allocation_verifier]")
{
	Allocator_Scope scope;
	Allocator       verifier;

	Alloc_Traits::allocate(verifier, 1);

	REQUIRE_THROWS_WITH(Allocator::handler()->cleanup(), memory_leaked);
}

TEST_CASE(
    "Monitor verifies deallocate address matches previous allocate address",
    "[allocation_verifier]")
{
	Allocator_Scope scope;
	Allocator       verifier;

	size_t count  = 2;
	auto  *memory = Alloc_Traits::allocate(verifier, count);

	Alloc_Traits::deallocate(verifier, memory + 1, count);

	REQUIRE_THROWS_WITH(
	    Allocator::handler()->cleanup(),
	    Catch::Matchers::ContainsSubstring(deallocating_unallocated_memory));
}

TEST_CASE(
    "Monitor verifiers deallocate count matches previous allocate count",
    "[allocation_verifier]")
{
	Allocator_Scope scope;
	Allocator       verifier;

	size_t count  = 2;
	auto  *memory = Alloc_Traits::allocate(verifier, count);

	Alloc_Traits::deallocate(verifier, memory, count + 1);

	REQUIRE_THROWS_WITH(
	    Allocator::handler()->cleanup(),
	    Catch::Matchers::ContainsSubstring(deallocating_count_mismatch));
}

TEST_CASE(
    "An allocate followed by a valid deallocate raises no error",
    "[allocation_verifier]")
{
	Allocator_Scope scope;
	Allocator       verifier;

	size_t count  = 1;
	auto  *memory = Alloc_Traits::allocate(verifier, count);

	Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_NOTHROW(Allocator::handler()->cleanup());
}

TEST_CASE("Allocations can only be deallocated once", "[allocation_verifier]")
{
	Allocator_Scope scope;
	Allocator       verifier;

	size_t count    = 2;
	auto  *memory_a = Alloc_Traits::allocate(verifier, count);
	auto  *memory_b = Alloc_Traits::allocate(verifier, count);

	Alloc_Traits::deallocate(verifier, memory_a, count);
	Alloc_Traits::deallocate(verifier, memory_a, count);
	Alloc_Traits::deallocate(verifier, memory_b, count);

	REQUIRE_THROWS_WITH(
	    Allocator::handler()->cleanup(),
	    deallocating_unallocated_memory);
}

TEST_CASE(
    "Multiple allocations each require a deallocation",
    "[allocation_verifier]")
{
	Allocator_Scope scope;
	Allocator       verifier;

	size_t count_a  = 1;
	auto  *memory_a = Alloc_Traits::allocate(verifier, count_a);
	size_t count_b  = 2;
	auto  *memory_b = Alloc_Traits::allocate(verifier, count_b);
	size_t count_c  = 3;
	Alloc_Traits::allocate(verifier, count_c);

	Alloc_Traits::deallocate(verifier, memory_a, count_a);
	Alloc_Traits::deallocate(verifier, memory_b, count_b);

	REQUIRE_THROWS_WITH(Allocator::handler()->cleanup(), memory_leaked);
}

TEST_CASE(
    "Deallocating constructed elements raises an error",
    "[allocation_verifier]")
{
	Allocator_Scope scope;
	Allocator       verifier;

	size_t count  = 1;
	auto  *memory = Alloc_Traits::allocate(verifier, count);

	Alloc_Traits::construct(verifier, memory, Empty_Value());
	Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_THROWS_WITH(Allocator::handler()->cleanup(), object_leaked);
}

TEST_CASE(
    "Constructed elements left undestroyed raise an error",
    "[allocation_verifier]")
{
	Allocator_Scope scope;
	Allocator       verifier;

	size_t count  = 1;
	auto  *memory = Alloc_Traits::allocate(verifier, count);

	std::uninitialized_default_construct_n(memory, count);

	Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_THROWS_WITH(Allocator::handler()->cleanup(), object_leaked);
}

TEST_CASE(
    "Calling construct on already constructed memory raises an error",
    "[allocation_verifier]")
{
	Allocator_Scope scope;
	Allocator       verifier;

	size_t count  = 1;
	auto  *memory = Alloc_Traits::allocate(verifier, count);

	Alloc_Traits::construct(verifier, memory, Empty_Value());
	Alloc_Traits::construct(verifier, memory, Empty_Value());

	Alloc_Traits::destroy(verifier, memory);
	Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_THROWS_WITH(Allocator::handler()->cleanup(), object_leaked);
}

TEST_CASE(
    "Objects initialised through a copy also require destruction",
    "[allocation_verifier]")
{
	Allocator_Scope scope;
	Allocator       verifier;

	size_t count  = 1;
	auto  *memory = Alloc_Traits::allocate(verifier, count);

	Allocator::Value value;
	std::uninitialized_fill_n(memory, 1, value);

	Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_THROWS_WITH(Allocator::handler()->cleanup(), object_leaked);
}

TEST_CASE(
    "Objects initialised through move construction also require destruction",
    "[allocation_verifier]")
{
	Allocator_Scope scope;
	Allocator       verifier;

	size_t count  = 1;
	auto  *memory = Alloc_Traits::allocate(verifier, count);

	Allocator::Value value;
	std::uninitialized_move_n(&value, 1, memory);

	Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_THROWS_WITH(Allocator::handler()->cleanup(), object_leaked);
}

TEST_CASE(
    "Copy construct from uninitialised memory raises an error"
    "[allocation_verifier]")
{
	Allocator_Scope scope;
	Allocator       verifier;

	size_t count    = 1;
	auto  *memory_a = Alloc_Traits::allocate(verifier, count);
	auto  *memory_b = Alloc_Traits::allocate(verifier, count);

	std::uninitialized_fill_n(memory_b, 1, *memory_a);

	Alloc_Traits::deallocate(verifier, memory_a, count);
	Alloc_Traits::deallocate(verifier, memory_b, count);

	REQUIRE_THROWS_WITH(
	    Allocator::handler()->cleanup(),
	    Catch::Matchers::ContainsSubstring(assign_from_uninitialized_memory));
}

TEST_CASE(
    "Move construct from uninitialised memory raises an error"
    "[allocation_verifier]")
{
	Allocator_Scope scope;
	Allocator       verifier;

	size_t count    = 1;
	auto  *memory_a = Alloc_Traits::allocate(verifier, count);
	auto  *memory_b = Alloc_Traits::allocate(verifier, count);

	std::uninitialized_move_n(memory_a, 1, memory_b);

	Alloc_Traits::deallocate(verifier, memory_a, count);
	Alloc_Traits::deallocate(verifier, memory_b, count);

	REQUIRE_THROWS_WITH(
	    Allocator::handler()->cleanup(),
	    Catch::Matchers::ContainsSubstring(assign_from_uninitialized_memory));
}

TEST_CASE(
    "Copy assign from uninitialised memory raises an error"
    "[allocation_verifier]")
{
	Allocator_Scope scope;
	Allocator       verifier;

	size_t count    = 1;
	auto  *memory_a = Alloc_Traits::allocate(verifier, count);
	auto  *memory_b = Alloc_Traits::allocate(verifier, count);
	Alloc_Traits::construct(verifier, memory_b);

	*memory_b = *memory_a;

	Alloc_Traits::deallocate(verifier, memory_a, count);
	Alloc_Traits::deallocate(verifier, memory_b, count);

	REQUIRE_THROWS_WITH(
	    Allocator::handler()->cleanup(),
	    Catch::Matchers::ContainsSubstring(assign_from_uninitialized_memory));
}

TEST_CASE(
    "Move assign from uninitialised memory raises an error"
    "[allocation_verifier]")
{
	Allocator_Scope scope;
	Allocator       verifier;

	size_t count    = 1;
	auto  *memory_a = Alloc_Traits::allocate(verifier, count);
	auto  *memory_b = Alloc_Traits::allocate(verifier, count);
	Alloc_Traits::construct(verifier, memory_b);

	*memory_b = std::move(*memory_a);

	Alloc_Traits::deallocate(verifier, memory_a, count);
	Alloc_Traits::deallocate(verifier, memory_b, count);

	REQUIRE_THROWS_WITH(
	    Allocator::handler()->cleanup(),
	    Catch::Matchers::ContainsSubstring(assign_from_uninitialized_memory));
}

TEST_CASE(
    "Assign to uninitialized memory raises an error",
    "[allocation_verifier]")
{
	Allocator_Scope scope;
	Allocator       verifier;

	size_t count  = 1;
	auto  *memory = Alloc_Traits::allocate(verifier, count);

	Allocator::Value value;
	*memory = value;

	Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_THROWS_WITH(
	    Allocator::handler()->cleanup(),
	    assign_uninitialized_memory);
}

TEST_CASE(
    "Performing move assign on uninitialized memory raises an error",
    "[allocation_verifier]")
{
	Allocator_Scope scope;
	Allocator       verifier;

	size_t count  = 1;
	auto  *memory = Alloc_Traits::allocate(verifier, 1);

	Allocator::Value value;
	*memory = std::move(value);

	Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_THROWS_WITH(
	    Allocator::handler()->cleanup(),
	    assign_uninitialized_memory);
}

TEST_CASE(
    "Calling destroy on unconstructed memory raises an error",
    "[allocation_verifier]")
{
	Allocator_Scope scope;
	Allocator       verifier;

	size_t count  = 1;
	auto  *memory = Alloc_Traits::allocate(verifier, count);

	Alloc_Traits::destroy(verifier, memory);

	Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_THROWS_WITH(
	    Allocator::handler()->cleanup(),
	    destroying_nonconstructed_memory);
}

TEST_CASE(
    "Objects move constructed from allocated memory require no destruction",
    "[allocation_verifier]")
{
	Allocator_Scope scope;
	Allocator       verifier;

	size_t count  = 1;
	auto  *memory = Alloc_Traits::allocate(verifier, count);
	Alloc_Traits::construct(verifier, memory, Empty_Value());

	Allocator::Value value(std::move(*memory));

	Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_NOTHROW(Allocator::handler()->cleanup());
}

TEST_CASE(
    "Objects move assigned from allocated memory require no destruction",
    "[allocation_verifier]")
{
	Allocator_Scope scope;
	Allocator       verifier;

	size_t count  = 1;
	auto  *memory = Alloc_Traits::allocate(verifier, count);
	Alloc_Traits::construct(verifier, memory, Empty_Value());

	Allocator::Value value;
	value = std::move(*memory);

	Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_NOTHROW(Allocator::handler()->cleanup());
}

TEST_CASE(
    "Objects copied from allocated memory still require desturction",
    "[allocation_verifier]")
{
	Allocator_Scope scope;
	Allocator       verifier;

	size_t count  = 1;
	auto  *memory = Alloc_Traits::allocate(verifier, count);

	Alloc_Traits::construct(verifier, memory, Empty_Value());
	Allocator::Value value(*memory);

	Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_THROWS_WITH(Allocator::handler()->cleanup(), object_leaked);
}

TEST_CASE(
    "Objects created through move assignment still require destruction",
    "[allocation_verifier]")
{
	Allocator_Scope scope;
	Allocator       verifier;

	size_t count  = 1;
	auto  *memory = Alloc_Traits::allocate(verifier, 1);

	Alloc_Traits::construct(verifier, memory, Empty_Value());
	Allocator::Value value;
	*memory = std::move(value);

	Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_THROWS_WITH(Allocator::handler()->cleanup(), object_leaked);
}

TEST_CASE(
    "A construct followed by a destroy raises no error",
    "[allocation_verifier]")
{
	Allocator_Scope scope;
	Allocator       verifier;

	size_t count  = 2;
	auto  *memory = Alloc_Traits::allocate(verifier, count);

	Alloc_Traits::construct(verifier, memory, Empty_Value());
	Alloc_Traits::construct(verifier, memory + 1, Empty_Value());

	Alloc_Traits::destroy(verifier, memory + 1);
	Alloc_Traits::destroy(verifier, memory);

	Alloc_Traits::deallocate(verifier, memory, count);

	REQUIRE_NOTHROW(Allocator::handler()->cleanup());
}

TEST_CASE(
    "Objects allocated on the stack do not raise errors",
    "[allocation_verifier]")
{
	Allocator_Scope scope;
	{
		Allocator        verifier;
		Allocator::Value value;
	}
	REQUIRE_NOTHROW(Allocator::handler()->cleanup());
}

TEST_CASE("allocated on the stack do not raise errors", "[allocation_verifier]")
{
	Allocator_Scope scope;
	{
		Allocator        verifier;
		Allocator::Value value_a;
		Allocator::Value value_b(std::move(value_a));
	}
	REQUIRE_NOTHROW(Allocator::handler()->cleanup());
}

} // namespace test
