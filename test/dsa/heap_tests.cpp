#include "matchers/equals_range_matcher.hpp"
#include "memory_monitor_handler_scope.hpp"
#include "utilities/allocation_verifier.hpp"
#include "utilities/memory_monitor_handler_scope.hpp"

#include <dsa/algorithms.hpp>
#include <dsa/heap.hpp>
#include <dsa/memory_monitor.hpp>

#include <string>

#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>

namespace test
{

struct IsHeap : Catch::Matchers::MatcherGenericBase
{
	bool match(auto const &heap) const {
		using Comparator =
		    typename std::decay_t<decltype(heap)>::Comparator;

		return dsa::is_heap(
		    std::begin(heap.storage()),
		    std::end(heap.storage()),
		    Comparator{});
	}

	std::string describe() const override {
		return "Is a heap";
	}
};

using Value         = int;
using Allocator     = dsa::Memory_Monitor<Value, Allocation_Verifier>;
using Handler_Scope = Memory_Monitor_Handler_Scope<Allocation_Verifier>;
using Heap          = dsa::Heap<Value, decltype(std::less{}), Allocator>;

TEST_CASE("Various mechanisims to initialise binary tree", "[heap]") {
	Handler_Scope scope;

	SECTION("Default initialised heap has no elements") {
		Heap heap;

		REQUIRE(heap.empty());
		REQUIRE(heap.size() == 0);
	}

	SECTION("Constructed from an initializer list") {
		Heap heap{0, 1, 2};

		REQUIRE(heap.size() == 3ULL);
		REQUIRE(heap.top() == 0);
		REQUIRE_THAT(heap, IsHeap());
	}
}

TEST_CASE("Heaps can be copied", "[heap]") {
	Handler_Scope scope;
	Heap          heap{0, 1, 2};

	SECTION("Heaps can be copy constructed from one another") {
		Heap copy(heap);

		REQUIRE(heap.storage() == copy.storage());
	}

	SECTION("Heaps can be copy assigned from one another") {
		Heap copy;
		REQUIRE(heap.storage() != copy.storage());

		copy = heap;
		REQUIRE(heap.storage() == copy.storage());
	}
}

TEST_CASE("Heaps can be moved", "[heap]") {
	Handler_Scope              scope;
	std::initializer_list<int> values{1, 2, 3};
	Heap                       temporary(values);

	SECTION("Heaps can be move constructed from one another") {
		Heap heap(std::move(temporary));

		REQUIRE_THAT(heap.storage(), EqualsRange(values));
	}

	SECTION("Heaps can be move assigned from one another") {
		Heap heap;
		REQUIRE(heap.storage() != temporary.storage());

		heap = std::move(temporary);
		REQUIRE_THAT(heap.storage(), EqualsRange(values));
	}
}

TEST_CASE("Heaps can be swapped", "[heap]") {
	Handler_Scope scope;

	std::initializer_list<int> values_a{1, 2, 3};
	std::initializer_list<int> values_b{4, 5, 6};

	Heap heap_a{values_a};
	Heap heap_b{values_b};

	swap(heap_a, heap_b);

	REQUIRE_THAT(heap_a.storage(), EqualsRange(values_b));
	REQUIRE_THAT(heap_b.storage(), EqualsRange(values_a));
}

TEST_CASE("Heaps can be inserted into", "[heap]") {
	Handler_Scope scope;

	SECTION("Inserting into an empty heap updates helper functions") {
		Heap heap;
		heap.push(0);

		REQUIRE_FALSE(heap.empty());
		REQUIRE(heap.size() == 1);
		REQUIRE(heap.top() == 0);
		REQUIRE_THAT(heap, IsHeap());
	}

	SECTION("Inserting multiple elements maintains min heap properties") {
		Heap heap;
		heap.push(1);
		heap.push(3);
		heap.push(5);
		heap.push(7);

		REQUIRE(heap.size() == 4ULL);
		REQUIRE(heap.top() == 1);
		REQUIRE_THAT(heap, IsHeap());
	}

	SECTION("Inserting new smaller element maintains min heap properties") {
		Heap heap{1, 3, 5};
		heap.push(2);

		REQUIRE(heap.size() == 4ULL);
		REQUIRE(heap.top() == 1);
		REQUIRE_THAT(heap, IsHeap());
	}

	SECTION("Inserting new smallest element updates root") {
		Heap heap{1, 2, 3, 4, 5, 6};
		heap.push(0);

		REQUIRE(heap.size() == 7ULL);
		REQUIRE(heap.top() == 0);
		REQUIRE_THAT(heap, IsHeap());
	}
}

TEST_CASE("Heaps can have their elements popped", "[heap]") {
	Handler_Scope scope;

	SECTION("Heap with a single element becomes empty") {
		Heap heap{0};
		heap.pop();

		REQUIRE(heap.empty());
		REQUIRE(heap.size() == 0);
	}

	SECTION("Pop replaces the element at the top") {
		Heap heap{0, 1};
		heap.pop();

		REQUIRE(heap.size() == 1);
		REQUIRE(heap.top() == 1);
	}

	SECTION("Pop chooses smallest(first) child") {
		Heap heap{0, 1, 2, 3};
		heap.pop();

		REQUIRE(heap.size() == 3);
		REQUIRE(heap.top() == 1);
		REQUIRE_THAT(heap, IsHeap());
	}

	SECTION("Pop chooses smallest(second) child") {
		Heap heap{0, 2, 1, 3};
		heap.pop();

		REQUIRE(heap.size() == 3);
		REQUIRE(heap.top() == 1);
		REQUIRE_THAT(heap, IsHeap());
	}

	SECTION("Pop maintains min heap property in deeper levels") {
		Heap heap{0, 2, 1, 3, 5, 7, 9};
		heap.pop();

		REQUIRE(heap.size() == 6);
		REQUIRE(heap.top() == 1);
		REQUIRE_THAT(heap, IsHeap());
	}

	// When we pop we 'reinsert' the last element into the tree, we do not
	// need to move this to the deepest level. We can stop when the property
	// is satisfied
	SECTION("'Reinserted' element can stop at a shallow level") {
		Heap heap{0, 1, 2, 6, 4, 5, 3};
		heap.pop();

		REQUIRE(heap.size() == 6);
		REQUIRE(heap.top() == 1);
		REQUIRE_THAT(heap, IsHeap());
	}
}

} // namespace test
