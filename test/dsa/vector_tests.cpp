#include "allocation_verifier.hpp"
#include "equals_range_matcher.hpp"
#include "memory_monitor_handler_scope.hpp"

#include <dsa/memory_monitor.hpp>
#include <dsa/vector.hpp>

#include <catch2/catch_all.hpp>

namespace test
{

template<typename T>
using Allocator_Base = dsa::Memory_Monitor<T, Allocation_Verifier>;

using Handler_Scope = Memory_Monitor_Handler_Scope<Allocation_Verifier>;

TEST_CASE(
    "Vector provides multiple constructors for easy initialisation",
    "[vector]") {
	using Vector = dsa::Vector<int, Allocator_Base>;
	Handler_Scope scope;

	constexpr size_t count         = 2;
	constexpr int    default_value = int{};
	constexpr int    value         = 5;

	SECTION("Default initialisation set size and capacity to zero") {
		Vector vector;

		REQUIRE(vector.empty());
		REQUIRE(vector.size() == 0);
		REQUIRE(vector.capacity() == 0);
	}

	SECTION(
	    "Custom size initialised vector with default constructed values") {
		Vector vector(count);

		REQUIRE_FALSE(vector.empty());
		REQUIRE(vector.size() == count);
		REQUIRE(vector.capacity() == count);
		REQUIRE_THAT(vector, EqualsRange({default_value, default_value}));
	}

	SECTION("Construct using custom size and custom value") {
		Vector vector(count, value);

		REQUIRE_FALSE(vector.empty());
		REQUIRE(vector.size() == count);
		REQUIRE_THAT(vector, EqualsRange({value, value}));
	}

	SECTION("Construct using list initialisation") {
		std::initializer_list<int> list{1, 2, 3};

		Vector vector(list);

		REQUIRE_FALSE(vector.empty());
		REQUIRE(vector.size() == list.size());
		REQUIRE(vector.capacity() == list.size());
		REQUIRE_THAT(vector, EqualsRange(list));
	}
}

TEST_CASE("Vectors can be compared", "[vectors]") {
	using Vector = dsa::Vector<int, Allocator_Base>;
	Handler_Scope scope;

	SECTION("Empty vectors are equal") {
		Vector lhs;
		Vector rhs;

		REQUIRE(lhs.empty());
		REQUIRE(lhs == rhs);
		REQUIRE_FALSE(lhs != rhs);
	}

	SECTION("Vectors differing is size are unequal") {
		Vector lhs(3ULL);
		Vector rhs(5ULL);

		REQUIRE_FALSE(lhs == rhs);
		REQUIRE(lhs != rhs);
	}

	SECTION("Vector with a differing element are unequal") {
		Vector lhs{1, 2, 2};
		Vector rhs{1, 2, 3};

		REQUIRE_FALSE(lhs == rhs);
		REQUIRE(lhs != rhs);
	}

	SECTION("Vectors with same size and element order are equal") {
		Vector lhs{1, 2, 3};
		Vector rhs{1, 2, 3};

		REQUIRE(lhs == rhs);
		REQUIRE_FALSE(lhs != rhs);
	}

	SECTION("Vectors capacity does not affect equality") {
		Vector lhs{1, 2, 3};
		Vector rhs{1, 2, 3};

		lhs.reserve(10ULL);

		REQUIRE(lhs == rhs);
		REQUIRE_FALSE(lhs != rhs);
	}
}

TEST_CASE("Vectors can be copied", "[vector]") {
	using Vector = dsa::Vector<int, Allocator_Base>;
	Handler_Scope scope;
	Vector        vector{1, 2, 2};

	SECTION("Vectors can be copy constructed") {
		Vector copy(vector);

		REQUIRE(vector == copy);
	}

	SECTION("Vectors can be copy assigned") {
		Vector copy{0};
		copy = vector;

		REQUIRE(vector == copy);
	}
}

TEST_CASE("Vectors can be moved", "[vector]") {
	using Vector = dsa::Vector<int, Allocator_Base>;
	Handler_Scope scope;

	std::initializer_list<int> list{1, 2, 3};
	Vector                     temporary{list};

	SECTION("Vectors can be move constructed") {
		Vector vector(std::move(temporary));

		REQUIRE(temporary.data() == nullptr);

		REQUIRE(vector.size() == list.size());
		REQUIRE_THAT(vector, EqualsRange(list));
	}

	SECTION("Vectors can be move assigned") {
		Vector vector{0};
		vector = std::move(temporary);

		REQUIRE(vector.size() == list.size());
		REQUIRE_THAT(vector, EqualsRange(list));
	}
}

TEST_CASE("Vectors can be swapped", "[vector]") {
	using Vector = dsa::Vector<int, Allocator_Base>;
	Handler_Scope scope;

	std::initializer_list<int> list_a{1, 2, 3};
	std::initializer_list<int> list_b{4, 5, 6};

	Vector vector_a{list_a};
	Vector vector_b{list_b};

	swap(vector_a, vector_b);

	REQUIRE_THAT(vector_a, EqualsRange(list_b));
	REQUIRE_THAT(vector_b, EqualsRange(list_a));
}

TEST_CASE("Vector provides an interface to access its elements", "[vector]") {
	using Vector = dsa::Vector<int, Allocator_Base>;
	Handler_Scope scope;

	constexpr int              front = 1;
	constexpr int              mid   = 1;
	constexpr int              back  = 1;
	std::initializer_list<int> list{front, mid, back};
	Vector                     vector{list};

	SECTION("Elements can be accessed through the subscript operator") {
		REQUIRE_THAT(vector, EqualsRange(list));
	}

	SECTION("The underlying buffer can be accessed through data") {
		REQUIRE(vector.data() == &vector[0]);
	}

	SECTION("Iterators can be access through the begin and end functions") {
		REQUIRE(vector.data() == vector.begin());
		REQUIRE(vector.data() + vector.size() == vector.end());
	}

	SECTION("The first element can be accessed through a function") {
		REQUIRE(vector.front() == front);

		vector.erase(0);

		REQUIRE(vector.front() == mid);
	}

	SECTION("The last element can be accessed through a function") {
		REQUIRE(vector.back() == back);

		vector.erase(list.size() - 1);

		REQUIRE(vector.back() == mid);
	}
}

TEST_CASE("Elements can be appended to the vector", "[vector]") {
	using Vector = dsa::Vector<int, Allocator_Base>;
	Handler_Scope scope;

	std::initializer_list<int> expected{3, 3, 2};

	Vector vector(2ULL, 3);
	vector.append(Vector::Value(2));

	REQUIRE(vector.size() == 3ULL);
	REQUIRE_THAT(vector, EqualsRange(expected));
}

TEST_CASE("Elements can be inserted into the vector", "[vector]") {
	using Vector = dsa::Vector<int, Allocator_Base>;
	Handler_Scope scope;

	Vector vector(4ULL, 2);

	SECTION("Insertion at the front shifts buffer elements backwards") {
		std::initializer_list<int> expected{3, 2, 2, 2, 2};

		vector.insert(0ULL, Vector::Value(3));

		REQUIRE(vector.size() == 5ULL);
		REQUIRE_THAT(vector, EqualsRange(expected));
	}

	SECTION("Insertion at the middle shifts later elements backwards") {
		std::initializer_list<int> expected{2, 2, 3, 2, 2};

		vector.insert(2ULL, Vector::Value(3));

		REQUIRE(vector.size() == 5ULL);
		REQUIRE_THAT(vector, EqualsRange(expected));
	}

	SECTION("Insertion at the end preserves earlier elements") {
		std::initializer_list<int> expected{2, 2, 2, 2, 3};

		vector.insert(vector.size(), Vector::Value(3));

		REQUIRE(vector.size() == 5ULL);
		REQUIRE_THAT(vector, EqualsRange(expected));
	}
}

TEST_CASE("Elements can be erased from the vector", "[vector]") {
	using Vector = dsa::Vector<int, Allocator_Base>;
	Handler_Scope scope;

	Vector vector{1, 2, 3};

	SECTION("Erasing the front element shifs buffer elements forward") {
		std::initializer_list<int> expected{2, 3};

		vector.erase(0ULL);

		REQUIRE(vector.size() == 2ULL);
		REQUIRE_THAT(vector, EqualsRange(expected));
	}

	SECTION("Erasing a middle element shifts later elements forward") {
		std::initializer_list<int> expected{1, 3};

		vector.erase(1ULL);

		REQUIRE(vector.size() == 2ULL);
		REQUIRE_THAT(vector, EqualsRange(expected));
	}

	SECTION("Erasing at the end preserves earlier elements") {
		std::initializer_list<int> expected{1, 2};

		vector.erase(2ULL);

		REQUIRE(vector.size() == 2ULL);
		REQUIRE_THAT(vector, EqualsRange(expected));
	}
}

TEST_CASE("Vectors can be shrunk to free unused memory", "[vector]") {
	using Vector = dsa::Vector<int, Allocator_Base>;
	Handler_Scope scope;

	std::initializer_list<int> list{1, 2, 3};

	Vector vector(list);

	vector.reserve(vector.capacity() * 2);

	REQUIRE(vector.size() < vector.capacity());

	vector.shrink_to_fit();

	REQUIRE(vector.size() == vector.capacity());
	REQUIRE_THAT(vector, EqualsRange(list));
}

TEST_CASE("Vectors can be cleared of all the current elements", "[vector]") {
	using Vector = dsa::Vector<int, Allocator_Base>;
	Handler_Scope scope;

	Vector vector{0, 1};
	vector.clear();

	REQUIRE(vector.empty());
	REQUIRE(vector.size() == 0);
}

TEST_CASE("Vectors can be resized", "[vector]") {
	using Vector = dsa::Vector<int, Allocator_Base>;
	Handler_Scope scope;

	Vector vector{1, 2, 3, 4};

	SECTION("Choosing a smaller size trmis excess elements") {
		std::initializer_list<int> expected{1, 2};

		vector.resize(expected.size());

		REQUIRE(vector.size() == expected.size());
		REQUIRE_THAT(vector, EqualsRange(expected));
	}

	SECTION("Choosing the same size leaves the vector unchanged") {
		std::initializer_list<int> expected{1, 2, 3, 4};

		vector.resize(expected.size());

		REQUIRE(vector.size() == expected.size());
		REQUIRE_THAT(vector, EqualsRange(expected));
	}

	SECTION("Chhosing a larger size default initialises new elements") {
		std::initializer_list<int> expected{1, 2, 3, 4, 0, 0};

		vector.resize(expected.size());

		REQUIRE(vector.size() == expected.size());
		REQUIRE_THAT(vector, EqualsRange(expected));
	}
}

} // namespace test
