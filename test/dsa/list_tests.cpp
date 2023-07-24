#include "allocation_verifier.hpp"
#include "equals_range_matcher.hpp"
#include "memory_monitor_handler_scope.hpp"

#include <dsa/list.hpp>
#include <dsa/memory_monitor.hpp>

#include <catch2/catch_all.hpp>

namespace test
{

using Value         = int;
using Allocator     = dsa::Memory_Monitor<Value, Allocation_Verifier>;
using Handler_Scope = Memory_Monitor_Handler_Scope<Allocation_Verifier>;
using List          = dsa::List<Value, Allocator>;

TEST_CASE("Various mechanisims to initialise list", "[list]") {
	Handler_Scope scope;

	SECTION("Default initialised list has no elements") {
		List list;

		REQUIRE(list.empty());
		REQUIRE(list.size() == 0);
	}

	SECTION("Construct using list initialisation") {
		std::initializer_list<int> values{1, 2, 3};

		List list(values);

		REQUIRE_FALSE(list.empty());
		REQUIRE_FALSE(list.size() == 0);
		REQUIRE_THAT(list, EqualsRange(values));
	}
}

TEST_CASE("Lists can be compared", "[list]") {
	Handler_Scope scope;

	SECTION("Empty lists are equal") {
		List lhs;
		List rhs;
		REQUIRE(lhs == rhs);
		REQUIRE_FALSE(lhs != rhs);
	}

	SECTION("Lists with differing sizes are unequal") {
		List lhs{1, 2};
		List rhs{1, 2, 3};
		REQUIRE_FALSE(lhs == rhs);
		REQUIRE(lhs != rhs);
	}

	SECTION("Lists with differing element are unequal") {
		List lhs{1, 2, 2};
		List rhs{1, 2, 3};
		REQUIRE_FALSE(lhs == rhs);
		REQUIRE(lhs != rhs);
	}

	SECTION("Lists with the same size and element order are equal") {
		List lhs{1, 2, 3};
		List rhs{1, 2, 3};
		REQUIRE(lhs == rhs);
		REQUIRE_FALSE(lhs != rhs);
	}
}

TEST_CASE("Lists can be copied", "[list]") {
	Handler_Scope scope;

	List list{1, 2, 2};

	SECTION("Lists can be copy constructed from another list") {
		List copy(list);
		REQUIRE(list == copy);
	}

	SECTION("Lists can be copy assigned from another list") {
		List copy;
		REQUIRE(list != copy);

		copy = list;
		REQUIRE(list == copy);
	}
}

TEST_CASE("Lists can be moved", "[list]") {
	Handler_Scope scope;

	std::initializer_list<int> values{1, 2, 3};
	List                       temporary{values};

	SECTION("Lists can be move constructed from another list") {
		List list(std::move(temporary));

		REQUIRE_THAT(list, EqualsRange(values));
	}

	SECTION("Lists can be move assigned from another list") {
		List list;
		list = std::move(temporary);

		REQUIRE_THAT(list, EqualsRange(values));
	}
}

TEST_CASE("Lists can be swapped", "[list]") {
	Handler_Scope scope;

	std::initializer_list<int> values_a{1, 2, 3};
	std::initializer_list<int> values_b{4, 5, 6};

	List list_a{values_a};
	List list_b{values_b};

	swap(list_a, list_b);

	REQUIRE_THAT(list_a, EqualsRange(values_b));
	REQUIRE_THAT(list_b, EqualsRange(values_a));
}

TEST_CASE("Lists provides an interface to access its elements", "[list]") {
	Handler_Scope scope;

	std::initializer_list<int> values{1, 2, 3};
	List                       list{values};

	REQUIRE(list.front() == data(values)[0]);

	list.detatch_front();

	REQUIRE(list.front() == data(values)[1]);
}

TEST_CASE("Elements can be inserted into the list", "[list]") {
	Handler_Scope scope;

	int                        value = 0;
	std::initializer_list<int> values{1, 2, 3};
	List                       list{values};

	SECTION("Elements can be added to the front with prepend") {
		std::initializer_list<int> expected_values{value, 1, 2, 3};
		list.prepend(value);

		REQUIRE(list.size() == 4);
		REQUIRE_THAT(list, EqualsRange(expected_values));
	}

	SECTION("Elements can be inserted at the front") {
		std::initializer_list<int> expected_values{value, 1, 2, 3};
		list.insert(0, value);

		REQUIRE(list.size() == 4);
		REQUIRE_THAT(list, EqualsRange(expected_values));
	}

	SECTION("Elements can be inserted into the middle") {
		std::initializer_list<int> expected_values{1, 2, value, 3};
		list.insert(2, value);

		REQUIRE(list.size() == 4);
		REQUIRE_THAT(list, EqualsRange(expected_values));
	}

	SECTION("Elements can be inserted at the back") {
		std::initializer_list<int> expected_values{1, 2, 3, value};
		list.insert(list.size(), value);

		REQUIRE(list.size() == 4);
		REQUIRE_THAT(list, EqualsRange(expected_values));
	}
}

TEST_CASE("Elements can be erased from the list", "[list]") {
	Handler_Scope scope;

	std::initializer_list<int> values{1, 2, 3};
	List                       list{values};

	SECTION("Front elements can be erased with detatch_front") {
		std::initializer_list<int> expected_values{2, 3};
		list.detatch_front();

		REQUIRE(list.size() == 2);
		REQUIRE_THAT(list, EqualsRange(expected_values));
	}

	SECTION("Elements can be erased at the front") {
		std::initializer_list<int> expected_values{2, 3};
		list.erase(0);

		REQUIRE(list.size() == 2);
		REQUIRE_THAT(list, EqualsRange(expected_values));
	}

	SECTION("Elements can be erased at the middle") {
		std::initializer_list<int> expected_values{1, 3};
		list.erase(1);

		REQUIRE(list.size() == 2);
		REQUIRE_THAT(list, EqualsRange(expected_values));
	}

	SECTION("Elements can be erased at the back") {
		std::initializer_list<int> expected_values{1, 2};
		list.erase(2);

		REQUIRE(list.size() == 2);
		REQUIRE_THAT(list, EqualsRange(expected_values));
	}

	SECTION("All elements can be erased with clear") {
		list.clear();

		REQUIRE(list.empty());
		REQUIRE(list.size() == 0);
	}
}

} // namespace test
