#include "dummy_pointer.hpp"
#include "dummy_value.hpp"
#include "empty_value.hpp"
#include "id.hpp"

#include <dsa/allocator_traits.hpp>
#include <dsa/default_allocator.hpp>

#include <catch2/catch.hpp>

TEST_CASE("Created test file", "[allocator_traits]")
{
	using Allocator = dsa::Default_Allocator<Empty_Value>;
	using Traits    = dsa::Allocator_Traits<Allocator>;

	STATIC_REQUIRE(std::is_same_v<Traits::Allocator, Allocator>);
	STATIC_REQUIRE(std::is_same_v<Traits::Value, Empty_Value>);
	STATIC_REQUIRE(std::is_same_v<Traits::Reference, Empty_Value &>);
	STATIC_REQUIRE(std::is_same_v<Traits::Const_Reference, Empty_Value const &>);
	STATIC_REQUIRE(std::is_same_v<Traits::Pointer, Empty_Value *>);
	STATIC_REQUIRE(std::is_same_v<Traits::Const_Pointer, Empty_Value const *>);
}

namespace
{

std::size_t max_limit_size_t()
{
	return std::numeric_limits<std::size_t>::max();
}

struct Dummy_Allocator
{
	using Value   = Dummy_Value;
	using Pointer = Dummy_Pointer;

	constexpr Pointer allocate(std::size_t count)
	{
		allocated_count = count;
		return Dummy_Pointer(Id(count));
	}

	constexpr void construct(
	    Pointer pointer,
	    Dummy_Value_Construct_Tag /* tag */)
	{
		constructed = pointer.id();
	}

	constexpr void deallocate(Pointer pointer, std::size_t count)
	{
		deallocated_count = count;
		deallocated       = pointer.id();
	}

	std::size_t allocated_count = 0ULL;
	Id          constructed;

	std::size_t deallocated_count = 0ULL;
	Id          deallocated;
};

using Traits = dsa::Allocator_Traits<Dummy_Allocator>;

constexpr std::pair<Dummy_Allocator, Dummy_Pointer> allocate(std::size_t count)
{
	Dummy_Allocator allocator;
	return {allocator, Traits::allocate(allocator, count)};
}

constexpr Dummy_Allocator construct(Id id)
{
	Dummy_Allocator allocator;
	Traits::construct(allocator, Dummy_Pointer(id), Dummy_Value_Construct_Tag());
	return allocator;
}

constexpr Dummy_Allocator deallocate(Id id, std::size_t count)
{
	Dummy_Allocator allocator;
	Traits::deallocate(allocator, Dummy_Pointer(id), count);
	return allocator;
}

} // namespace

TEST_CASE(
    "Allocator_Traits calls custom allocator operation overloads if present",
    "[allocator_traits]")
{
	SECTION("Custom allocate is called in a static context")
	{
		constexpr std::size_t count = 5;
		constexpr auto        pair  = allocate(count);

		STATIC_REQUIRE(pair.first.allocated_count == count);
		STATIC_REQUIRE(pair.second.id().value() == count);
	}

	SECTION("Custom allocate is called with the forwarded count")
	{
		const std::size_t count   = GENERATE(0ULL, max_limit_size_t());
		auto [allocator, pointer] = allocate(count);
		REQUIRE(allocator.allocated_count == count);
		REQUIRE(pointer.id().value() == count);
	}

	SECTION("Custom construct is called in a static context")
	{
		constexpr Id              id        = Id(7);
		constexpr Dummy_Allocator allocator = construct(id);

		STATIC_REQUIRE(allocator.constructed == id);
	}

	SECTION("Custom construct is called with the forwarded arguments")
	{
		const Id        id(GENERATE(0ULL, max_limit_size_t()));
		Dummy_Allocator allocator = construct(id);

		REQUIRE(allocator.constructed == id);
	}

	SECTION("Custom deallocate is called in a static context")
	{
		constexpr std::size_t count = 3;
		constexpr Id          id    = Id(9);

		constexpr Dummy_Allocator allocator =
		    deallocate(id, count);

		STATIC_REQUIRE(allocator.deallocated_count == count);
		STATIC_REQUIRE(allocator.deallocated == id);
	}

	SECTION("Custom deallocate is called with the forwarded arguments")
	{
		const std::size_t count = GENERATE(0ULL, max_limit_size_t());
		const Id          id(GENERATE(0ULL, max_limit_size_t()));

		auto allocator = deallocate(id, count);

		REQUIRE(allocator.deallocated_count == count);
		REQUIRE(allocator.deallocated == id);
	}
}
