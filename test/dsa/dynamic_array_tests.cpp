#include "allocation_verifier.hpp"
#include "empty_value.hpp"
#include "equals_range_matcher.hpp"
#include "memory_monitor_handler_scope.hpp"

#include <dsa/dynamic_array.hpp>
#include <dsa/memory_monitor.hpp>

#include <catch2/catch_all.hpp>

namespace test
{

using Value         = int;
using Allocator     = dsa::Memory_Monitor<Value, Allocation_Verifier>;
using Handler_Scope = Memory_Monitor_Handler_Scope<Allocation_Verifier>;
using Dynamic_Array = dsa::Dynamic_Array<Value, Allocator>;

TEST_CASE("Various mechanisims to initialise dynamic array", "[dynamic_array]") {
	Handler_Scope scope;

	SECTION("Default initialisation sets size to zero") {
		Dynamic_Array array;

		REQUIRE(array.size() == 0);
	}

	SECTION(
	    "Custom size initialises array with default constructed values") {
		constexpr size_t count = 2;

		Dynamic_Array array(count);

		REQUIRE(array.size() == count);
		REQUIRE_THAT(array, EqualsRange({int{}, int{}}));
	}

	SECTION("Construct using custom size and custom value") {
		constexpr size_t count = 2;
		constexpr int    value = 5;

		Dynamic_Array array(count, value);

		REQUIRE(array.size() == count);
		REQUIRE_THAT(array, EqualsRange({value, value}));
	}

	SECTION("Construct using list initialisation") {
		std::initializer_list<int> list{1, 2, 3};

		Dynamic_Array array(list);

		REQUIRE(array.size() == list.size());
		REQUIRE_THAT(array, EqualsRange(list));
	}
}

TEST_CASE("Dynamic arrays can be compared", "[dynamic_array]") {
	Handler_Scope scope;

	SECTION("Empty arrays are equal") {
		Dynamic_Array lhs;
		Dynamic_Array rhs;
		REQUIRE(lhs == rhs);
		REQUIRE_FALSE(lhs != rhs);
	}

	SECTION("Arrays differing is size are unequal") {
		Dynamic_Array lhs(3ULL);
		Dynamic_Array rhs(5ULL);
		REQUIRE_FALSE(lhs == rhs);
		REQUIRE(lhs != rhs);
	}

	SECTION("Arrays with a differing element are unequal") {
		Dynamic_Array lhs{1, 2, 2};
		Dynamic_Array rhs{1, 2, 3};
		REQUIRE_FALSE(lhs == rhs);
		REQUIRE(lhs != rhs);
	}

	SECTION("Arrays with same size and element order are equal") {
		Dynamic_Array lhs{1, 2, 3};
		Dynamic_Array rhs{1, 2, 3};
		REQUIRE(lhs == rhs);
		REQUIRE_FALSE(lhs != rhs);
	}
}

TEST_CASE("Dynamic arrays can be copied", "[dynamic_array]") {
	Handler_Scope scope;

	Dynamic_Array array{1, 2, 2};

	SECTION("Arrays can be copy constructed from another array") {
		Dynamic_Array copy(array);
		REQUIRE(array == copy);
	}

	SECTION("Arrays can be copy assigned from another array") {
		Dynamic_Array copy{0};
		copy = array;
		REQUIRE(array == copy);
	}
}

TEST_CASE("Dynamic arrays can be moved", "[dynamic_array]") {
	Handler_Scope scope;

	std::initializer_list<int> list{1, 2, 3};
	Dynamic_Array              temporary{list};

	SECTION("Arrays can be move constructed from another array") {
		Dynamic_Array array(std::move(temporary));

		REQUIRE(temporary.data() == nullptr);
		REQUIRE_THAT(array, EqualsRange(list));
	}

	SECTION("Arrays can be move assigned from another array") {
		Dynamic_Array array{0};
		array = std::move(temporary);

		REQUIRE_THAT(array, EqualsRange(list));
	}
}

TEST_CASE("Dynamic arrays can be swapped", "[dynamic_array]") {
	Handler_Scope scope;

	std::initializer_list<int> list_a{1, 2, 3};
	std::initializer_list<int> list_b{4, 5, 6};

	Dynamic_Array array_a{list_a};
	Dynamic_Array array_b{list_b};

	swap(array_a, array_b);

	REQUIRE_THAT(array_a, EqualsRange(list_b));
	REQUIRE_THAT(array_b, EqualsRange(list_a));
}

TEST_CASE(
    "Dynamic array provides an interface to access its elements",
    "[dynamic_array]") {
	Handler_Scope scope;

	std::initializer_list<int> list{1, 2, 3};
	Dynamic_Array              array{list};

	SECTION("Elements can be accessed throught the subscript operator") {
		REQUIRE_THAT(array, EqualsRange(list));
	}

	SECTION("The underlying buffer can be accessed through data") {
		REQUIRE(array.data() == &array[0]);
	}

	SECTION("Iterators can be access through the begin and end functions") {
		REQUIRE(&array.data()[0] == array.begin());
		REQUIRE(&array.data()[0] + array.size() == array.end());
	}
}

TEST_CASE("Dynamic arrays can be resized at runtime", "[dynamic_array]") {
	Handler_Scope scope;

	SECTION("Reducing array size preserves the kept elements") {
		std::initializer_list<int> list{1, 2, 3};
		std::initializer_list<int> expected{data(list)[0], data(list)[1]};

		Dynamic_Array array{list};
		array.resize(expected.size());

		REQUIRE_THAT(array, EqualsRange(expected));
	}

	SECTION("Increasing array size default initialises new elements") {
		std::initializer_list<int> list{1, 2};
		std::initializer_list<int> expected{
		    data(list)[0],
		    data(list)[1],
		    int{},
		    int{}};

		Dynamic_Array array{list};
		array.resize(expected.size());

		REQUIRE_THAT(array, EqualsRange(expected));
	}

	SECTION(
	    "Increasing array size initialises new elements to given value") {
		Dynamic_Array::Value       value(10);
		std::initializer_list<int> list{1, 2};
		std::initializer_list<int> expected{
		    data(list)[0],
		    data(list)[1],
		    value,
		    value};

		Dynamic_Array array{list};
		array.resize(expected.size(), value);

		REQUIRE_THAT(array, EqualsRange(expected));
	}
}

} // namespace test
