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

} // namespace test
