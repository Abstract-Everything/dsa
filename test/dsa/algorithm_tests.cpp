#include "allocation_verifier.hpp"
#include "memory_monitor_handler_scope.hpp"
#include "incomparable_value.hpp"

#include <dsa/algorithms.hpp>
#include <dsa/dynamic_array.hpp>
#include <dsa/memory_monitor.hpp>

#include <compare>

#include <catch2/catch_all.hpp>

namespace test
{

// TODO: Stop using this for sort algorithms as this does not provide a total
// order
struct Smaller_By_Two
{
	auto operator()(int lhs, int rhs) const -> bool {
		int difference = lhs - rhs;
		return difference >= 0 && difference <= 2;
	}
};

TEST_CASE("Validate min heap stored in an array", "[algorithms]") {
	auto is_min_heap = [](dsa::Dynamic_Array<int> const &array) -> bool {
		return dsa::is_heap(array.begin(), array.end(), std::less{});
	};

	SECTION("An empty array is a valid heap") {
		dsa::Dynamic_Array<int> array;
		REQUIRE(is_min_heap(array));
	}

	SECTION("A single element is a valid heap") {
		dsa::Dynamic_Array<int> array{1};
		REQUIRE(is_min_heap(array));
	}

	SECTION("An element followed by two larger elements is a valid heap") {
		dsa::Dynamic_Array<int> array{0, 1, 2};
		REQUIRE(is_min_heap(array));
	}

	SECTION("An element followed a smaller element is not a heap") {
		dsa::Dynamic_Array<int> array{2, 1};
		REQUIRE_FALSE(is_min_heap(array));
	}

	SECTION("Validate a heap with a depth of three") {
		dsa::Dynamic_Array<int>
		    array{0, 10, 1'000, 20, 30, 2'000, 3'000, 25, 30, 33, 32};
		REQUIRE(is_min_heap(array));
	}

	SECTION("Reject an invalid heap ending with a small element") {
		dsa::Dynamic_Array<int>
		    array{0, 10, 1'000, 20, 30, 2'000, 3'000, 25, 30, 33, 32, 1'999};
		REQUIRE_FALSE(is_min_heap(array));
	}

	SECTION("Arbitrary comparators can be used") {
		SECTION(
		    "A decreasing sequence is not a min heap but it is a max "
		    "heap") {
			dsa::Dynamic_Array<int> array{0, -1, -2, -3};
			REQUIRE_FALSE(
			    dsa::is_heap(array.begin(), array.end(), std::less{}));
			REQUIRE(dsa::is_heap(
			    array.begin(),
			    array.end(),
			    std::greater{}));
		}

		SECTION(
		    "Check that following elements are up to two units less") {
			SECTION("Valid sequence") {
				dsa::Dynamic_Array<int> array{10, 9, 8, 8, 7};
				REQUIRE(dsa::is_heap(
				    array.begin(),
				    array.end(),
				    Smaller_By_Two{}));
			}

			SECTION("Invalid last element") {
				dsa::Dynamic_Array<int> array{10, 9, 8, 6};
				REQUIRE_FALSE(dsa::is_heap(
				    array.begin(),
				    array.end(),
				    Smaller_By_Two{}));
			}
		}
	}
}

TEST_CASE("is_sorted validates that a range is sorted", "[algorithms]") {
	SECTION("An empty array is sorted") {
		dsa::Dynamic_Array<int> array;

		REQUIRE(dsa::is_sorted(array.begin(), array.end()));
	}

	SECTION("A single element is sorted") {
		dsa::Dynamic_Array array{0};

		REQUIRE(dsa::is_sorted(array.begin(), array.end()));
	}

	SECTION("Two unsorted elements are unsorted") {
		dsa::Dynamic_Array array{1, 0};

		REQUIRE_FALSE(dsa::is_sorted(array.begin(), array.end()));
	}

	SECTION("Two sorted elements are sorted") {
		dsa::Dynamic_Array array{0, 1};

		REQUIRE(dsa::is_sorted(array.begin(), array.end()));
	}

	SECTION("An increasing sequence is sorted") {
		dsa::Dynamic_Array array{0, 3, 5, 11, 15, 20, 30};

		REQUIRE(dsa::is_sorted(array.begin(), array.end()));
	}

	SECTION("Elements at the end boundary are checked") {
		dsa::Dynamic_Array array{0, 3, 5, 11, 15, 20, 30, 1};

		REQUIRE_FALSE(dsa::is_sorted(array.begin(), array.end()));
	}

	SECTION("Arbitrary comparators can be used") {
		SECTION("Can check if array is sorted in descending order") {
			dsa::Dynamic_Array array{30, 25, 14, 4, 1};

			REQUIRE(dsa::is_sorted(
			    array.begin(),
			    array.end(),
			    std::greater{}));
		}

		SECTION("Valid sequence") {
			dsa::Dynamic_Array<int> array{10, 9, 7, 5};
			REQUIRE(dsa::is_sorted(
			    array.begin(),
			    array.end(),
			    Smaller_By_Two{}));
		}

		SECTION("Invalid last element") {
			dsa::Dynamic_Array<int> array{10, 9, 7, 4};
			REQUIRE_FALSE(dsa::is_sorted(
			    array.begin(),
			    array.end(),
			    Smaller_By_Two{}));
		}
	}
}

TEST_CASE("Insertion sort correctly sorts an array", "[algorithms]") {
	SECTION("An empty array is already sorted") {
		dsa::Dynamic_Array<int> array;

		dsa::insertion_sort(array.begin(), array.end());

		REQUIRE(dsa::is_sorted(array.begin(), array.end()));
	}

	SECTION("A single element is already sorted") {
		dsa::Dynamic_Array array{0};

		dsa::insertion_sort(array.begin(), array.end());

		REQUIRE(dsa::is_sorted(array.begin(), array.end()));
	}

	SECTION("Two sorted elements are unmodified") {
		dsa::Dynamic_Array array{0, 1};

		dsa::insertion_sort(array.begin(), array.end());

		REQUIRE(dsa::is_sorted(array.begin(), array.end()));
	}

	SECTION("Two unsorted elements is sorted") {
		dsa::Dynamic_Array array{1, 0};

		dsa::insertion_sort(array.begin(), array.end());

		REQUIRE(dsa::is_sorted(array.begin(), array.end()));
	}

	SECTION(
	    "Sorted elements are maintained by moving new elements into the "
	    "correct position") {
		dsa::Dynamic_Array array{0, 2, 3, 4, 1};

		dsa::insertion_sort(array.begin(), array.end());

		REQUIRE(dsa::is_sorted(array.begin(), array.end()));
	}

	SECTION("Multiple elements are correctly sorted") {
		dsa::Dynamic_Array array{9, 3, 8, 2, 1, 7, 5, 6, 4, 10};

		dsa::insertion_sort(array.begin(), array.end());

		REQUIRE(dsa::is_sorted(array.begin(), array.end()));
	}

	SECTION("Arbitrary comparators can be used") {
		SECTION("Can sort an array in descending order") {
			dsa::Dynamic_Array array{9, 3, 8, 2, 1, 7, 5, 6, 4, 10};

			dsa::insertion_sort(
			    array.begin(),
			    array.end(),
			    std::greater{});

			REQUIRE(dsa::is_sorted(
			    array.begin(),
			    array.end(),
			    std::greater{}));
		}

		SECTION("Subsequent elements are two units less") {
			dsa::Dynamic_Array array{9, 3, 8, 2, 1, 7, 5, 6, 4, 10};

			dsa::insertion_sort(
			    array.begin(),
			    array.end(),
			    std::greater{});

			REQUIRE(dsa::is_sorted(
			    array.begin(),
			    array.end(),
			    Smaller_By_Two{}));
		}

		SECTION("Invalid last element") {
			dsa::Dynamic_Array<int> array{10, 9, 7, 4};
			REQUIRE_FALSE(dsa::is_sorted(
			    array.begin(),
			    array.end(),
			    Smaller_By_Two{}));
		}
	}
}

TEST_CASE("Selection sort correctly sorts an array", "[algorithms]") {
	SECTION("An empty array is already sorted") {
		dsa::Dynamic_Array<int> array;

		dsa::selection_sort(array.begin(), array.end());

		REQUIRE(dsa::is_sorted(array.begin(), array.end()));
	}

	SECTION("A single element is already sorted") {
		dsa::Dynamic_Array array{0};

		dsa::selection_sort(array.begin(), array.end());

		REQUIRE(dsa::is_sorted(array.begin(), array.end()));
	}

	SECTION("Two sorted elements are unmodified") {
		dsa::Dynamic_Array array{0, 1};

		dsa::selection_sort(array.begin(), array.end());

		REQUIRE(dsa::is_sorted(array.begin(), array.end()));
	}

	SECTION("Two unsorted elements is sorted") {
		dsa::Dynamic_Array array{1, 0};

		dsa::selection_sort(array.begin(), array.end());

		REQUIRE(dsa::is_sorted(array.begin(), array.end()));
	}

	SECTION("Multiple elements are correctly sorted") {
		dsa::Dynamic_Array array{9, 3, 8, 2, 1, 7, 5, 6, 4, 10};

		dsa::selection_sort(array.begin(), array.end());

		REQUIRE(dsa::is_sorted(array.begin(), array.end()));
	}

	SECTION("Arbitrary comparators can be used") {
		SECTION("Can sort an array in descending order") {
			dsa::Dynamic_Array array{9, 3, 8, 2, 1, 7, 5, 6, 4, 10};

			dsa::selection_sort(
			    array.begin(),
			    array.end(),
			    std::greater{});

			REQUIRE(dsa::is_sorted(
			    array.begin(),
			    array.end(),
			    std::greater{}));
		}

		SECTION("Subsequent elements are two units less") {
			dsa::Dynamic_Array array{9, 3, 8, 2, 1, 7, 5, 6, 4, 10};

			dsa::selection_sort(
			    array.begin(),
			    array.end(),
			    std::greater{});

			REQUIRE(dsa::is_sorted(
			    array.begin(),
			    array.end(),
			    Smaller_By_Two{}));
		}

		SECTION("Invalid last element") {
			dsa::Dynamic_Array<int> array{10, 9, 7, 4};
			REQUIRE_FALSE(dsa::is_sorted(
			    array.begin(),
			    array.end(),
			    Smaller_By_Two{}));
		}
	}
}

TEST_CASE("Linear search finds first occurence of element", "[algorithms]") {
	SECTION("Search does not find element in empty array") {
		dsa::Dynamic_Array<int> array;

		auto element = dsa::linear_search(array.begin(), array.end(), 0);

		REQUIRE_FALSE(element.has_value());
	}

	SECTION("Search does not find absent element in a single element array") {
		dsa::Dynamic_Array array{0};

		auto element = dsa::linear_search(array.begin(), array.end(), 1);

		REQUIRE_FALSE(element.has_value());
	}

	SECTION("Search finds element in a single element array") {
		dsa::Dynamic_Array array{0};

		auto element = dsa::linear_search(array.begin(), array.end(), 0);

		REQUIRE(element.has_value());
		REQUIRE(element.value() == array.begin());
	}

	SECTION("Search does not find absent element in a multi-element array") {
		dsa::Dynamic_Array array{53, 31, 45, 21, 33};

		auto element = dsa::linear_search(array.begin(), array.end(), 44);

		REQUIRE_FALSE(element.has_value());
	}

	SECTION("Search finds element in a multi-element array") {
		dsa::Dynamic_Array array{53, 31, 45, 21, 33};

		auto element = dsa::linear_search(array.begin(), array.end(), 21);

		REQUIRE(element.has_value());
		REQUIRE(element.value() == array.end() - 2);
	}

	SECTION("Search finds last element in a multi-element array") {
		dsa::Dynamic_Array array{53, 31, 45, 21, 33};

		auto element = dsa::linear_search(array.begin(), array.end(), 33);

		REQUIRE(element.has_value());
		REQUIRE(element.value() == array.end() - 1);
	}

	SECTION("Search supports a predicate argument") {
		dsa::Dynamic_Array array{
		    Incomparable_Value(9),
		    Incomparable_Value(3),
		    Incomparable_Value(10),
		    Incomparable_Value(7),
		    Incomparable_Value(2),
		    Incomparable_Value(6),
		};

		auto element = dsa::linear_search(
		    array.begin(),
		    array.end(),
		    [](Incomparable_Value const &value) {
			    return value.compare(Incomparable_Value(9));
		    });

		REQUIRE(element.has_value());
		REQUIRE(element.value() == array.begin());
	}
}

} // namespace test
