#include "allocation_verifier.hpp"
#include "memory_monitor_handler_scope.hpp"

#include <dsa/algorithms.hpp>
#include <dsa/dynamic_array.hpp>
#include <dsa/memory_monitor.hpp>

#include <compare>

#include <catch2/catch_all.hpp>

namespace test
{

TEST_CASE("Validate min heap stored in an array", "[algorithms]")
{
	auto is_min_heap = [](dsa::Dynamic_Array<int> const &array) -> bool
	{ return dsa::is_heap(array.begin(), array.end(), std::less{}); };

	SECTION("An empty array is a valid heap")
	{
		dsa::Dynamic_Array<int> array;
		REQUIRE(is_min_heap(array));
	}

	SECTION("A single element is a valid heap")
	{
		dsa::Dynamic_Array<int> array{1};
		REQUIRE(is_min_heap(array));
	}

	SECTION("An element followed by two larger elements is a valid heap")
	{
		dsa::Dynamic_Array<int> array{0, 1, 2};
		REQUIRE(is_min_heap(array));
	}

	SECTION("An element followed a smaller element is not a heap")
	{
		dsa::Dynamic_Array<int> array{2, 1};
		REQUIRE_FALSE(is_min_heap(array));
	}

	SECTION("Validate a heap with a depth of three")
	{
		dsa::Dynamic_Array<int>
		    array{0, 10, 1000, 20, 30, 2000, 3000, 25, 30, 33, 32};
		REQUIRE(is_min_heap(array));
	}

	SECTION("Reject an invalid heap ending with a small element")
	{
		dsa::Dynamic_Array<int>
		    array{0, 10, 1000, 20, 30, 2000, 3000, 25, 30, 33, 32, 1999};
		REQUIRE_FALSE(is_min_heap(array));
	}
}

TEST_CASE("Validate heap given a custom comparator", "[algorithms]")
{
	SECTION("A decreasing sequence is not a min heap but it is a max heap")
	{
		dsa::Dynamic_Array<int> array{0, -1, -2, -3};
		REQUIRE_FALSE(
		    dsa::is_heap(array.begin(), array.end(), std::less{}));
		REQUIRE(dsa::is_heap(array.begin(), array.end(), std::greater{}));
	}

	SECTION("Check that following elements are up to two units less")
	{
		struct Comparator
		{
			auto operator()(int lhs, int rhs) const -> bool
			{
				int difference = lhs - rhs;
				return difference >= 0 && difference <= 2;
			}
		};

		SECTION("Valid sequence")
		{
			dsa::Dynamic_Array<int> array{10, 9, 8, 8, 7};
			REQUIRE(
			    dsa::is_heap(array.begin(), array.end(), Comparator{}));
		}

		SECTION("Invalid last element")
		{
			dsa::Dynamic_Array<int> array{10, 9, 8, 6};
			REQUIRE_FALSE(
			    dsa::is_heap(array.begin(), array.end(), Comparator{}));
		}
	}
}

} // namespace test
