#include "allocation_verifier.hpp"
#include "incomparable_value.hpp"
#include "memory_monitor_handler_scope.hpp"

#include <dsa/algorithms.hpp>
#include <dsa/memory.hpp>
#include <dsa/dynamic_array.hpp>
#include <dsa/memory_monitor.hpp>

#include <compare>
#include <memory>

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

TEST_CASE("Merge sort correctly sorts an array", "[algorithms]") {
	SECTION("An empty array is already sorted") {
		dsa::Dynamic_Array<int> array;

		dsa::merge_sort(array.begin(), array.end());

		REQUIRE(dsa::is_sorted(array.begin(), array.end()));
	}

	SECTION("A single element is already sorted") {
		dsa::Dynamic_Array array{0};

		dsa::merge_sort(array.begin(), array.end());

		REQUIRE(dsa::is_sorted(array.begin(), array.end()));
	}

	SECTION("Two sorted elements are unmodified") {
		dsa::Dynamic_Array array{0, 1};

		dsa::merge_sort(array.begin(), array.end());

		REQUIRE(dsa::is_sorted(array.begin(), array.end()));
	}

	SECTION("Two unsorted elements is sorted") {
		dsa::Dynamic_Array array{1, 0};

		dsa::merge_sort(array.begin(), array.end());

		REQUIRE(dsa::is_sorted(array.begin(), array.end()));
	}

	SECTION("Multiple elements are correctly sorted") {
		dsa::Dynamic_Array array{9, 3, 8, 2, 1, 7, 5, 6, 4, 10};

		dsa::merge_sort(array.begin(), array.end());

		REQUIRE(dsa::is_sorted(array.begin(), array.end()));
	}

	SECTION("Arbitrary comparators can be used") {
		SECTION("Can sort an array in descending order") {
			dsa::Dynamic_Array array{9, 3, 8, 2, 1, 7, 5, 6, 4, 10};

			dsa::merge_sort(
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

			dsa::merge_sort(
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

TEST_CASE("Binary search searches an array for an element", "[algorithms]") {
	SECTION("Search does not find element in empty array") {
		dsa::Dynamic_Array<int> array;

		auto element = dsa::binary_search(array.begin(), array.end(), 0);

		REQUIRE_FALSE(element.has_value());
	}

	SECTION("Search does not find absent element in a single element array") {
		dsa::Dynamic_Array array{0};

		auto element = dsa::binary_search(array.begin(), array.end(), 1);

		REQUIRE_FALSE(element.has_value());
	}

	SECTION("Search finds element in a single element array") {
		dsa::Dynamic_Array array{0};

		auto element = dsa::binary_search(array.begin(), array.end(), 0);

		REQUIRE(element.has_value());
		REQUIRE(element.value() == array.begin());
	}

	SECTION("Search does not find absent element in a multi-element array") {
		dsa::Dynamic_Array array{21, 31, 33, 45, 53};

		auto element = dsa::binary_search(array.begin(), array.end(), 44);

		REQUIRE_FALSE(element.has_value());
	}

	SECTION("Search finds element in a multi-element array") {
		dsa::Dynamic_Array array{21, 31, 33, 45, 53};

		auto element = dsa::binary_search(array.begin(), array.end(), 45);

		REQUIRE(element.has_value());
		REQUIRE(element.value() == array.end() - 2);
	}

	SECTION("Search finds last element in a multi-element array") {
		dsa::Dynamic_Array array{21, 31, 33, 45, 53};

		auto element = dsa::binary_search(array.begin(), array.end(), 53);

		REQUIRE(element.has_value());
		REQUIRE(element.value() == array.end() - 1);
	}

	SECTION("Search finds first element in a multi-element array") {
		dsa::Dynamic_Array array{21, 31, 33, 45, 53};

		auto element = dsa::binary_search(array.begin(), array.end(), 21);

		REQUIRE(element.has_value());
		REQUIRE(element.value() == array.begin());
	}

	SECTION("Search supports a predicate argument") {
		dsa::Dynamic_Array array{
		    Incomparable_Value(2),
		    Incomparable_Value(3),
		    Incomparable_Value(6),
		    Incomparable_Value(7),
		    Incomparable_Value(9),
		    Incomparable_Value(10),
		};

		auto element = dsa::binary_search(
		    array.begin(),
		    array.end(),
		    [](Incomparable_Value const &value) {
			    return value.compare(Incomparable_Value(2));
		    });

		REQUIRE(element.has_value());
		REQUIRE(element.value() == array.begin());
	}
}

TEST_CASE(
    "Sum components search find two elements adding up to a sum",
    "[algorithms]") {
	SECTION("Sufficient elements must be present within the array") {
		SECTION("Empty array") {
			dsa::Dynamic_Array<int> array;

			auto pair = dsa::sum_components_search(
			    array.begin(),
			    array.end(),
			    0);

			REQUIRE_FALSE(pair.has_value());
		}

		SECTION("Single element array") {
			dsa::Dynamic_Array array{1};

			auto pair = dsa::sum_components_search(
			    array.begin(),
			    array.end(),
			    0);

			REQUIRE_FALSE(pair.has_value());
		}
	}

	SECTION("Two distinct elements must add up to the value") {
		SECTION("Two distinct element satisfy the condition")
		{
			dsa::Dynamic_Array array{1, 1};

			auto pair = dsa::sum_components_search(
			    array.begin(),
			    array.end(),
			    2);

			REQUIRE(pair.has_value());
			REQUIRE(
			    ((pair.value().first == array.begin()
			      && pair.value().second == array.begin() + 1)
			     || (pair.value().first == array.begin() + 1
				 && pair.value().second == array.begin())));
		}

		SECTION("Sum requires two distinct elements")
		{
			dsa::Dynamic_Array array{2};

			auto pair = dsa::sum_components_search(
			    array.begin(),
			    array.end(),
			    2);

			REQUIRE_FALSE(pair.has_value());
		}

		SECTION("Elements must add up to the required sum")
		{
			dsa::Dynamic_Array array{1, 1};

			auto pair = dsa::sum_components_search(
			    array.begin(),
			    array.end(),
			    3);

			REQUIRE_FALSE(pair.has_value());
		}
	}

	SECTION("A sum is found in a multi-element array")
	{
		dsa::Dynamic_Array array{7, 4, 3, 9};

		auto pair =
		    dsa::sum_components_search(array.begin(), array.end(), 7);

		REQUIRE(pair.has_value());
		REQUIRE(pair.value().first == array.begin() + 2);
		REQUIRE(pair.value().second == array.begin() + 1);
	}

	SECTION("No sum is found in a multi-element array")
	{
		dsa::Dynamic_Array array{7, 4, 3, 9};

		auto pair =
		    dsa::sum_components_search(array.begin(), array.end(), 14);

		REQUIRE_FALSE(pair.has_value());
	}
}

TEST_CASE("Checks if two iterator ranges overlap", "[algorithms]") {
	dsa::Dynamic_Array range{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	SECTION("No overlap for destination range before source range") {
		REQUIRE_FALSE(dsa::iterators_overlap(range.begin() + 5, range.end(), range.begin()));
	}

	SECTION("Overlap if destination range end with source begin") {
		REQUIRE(dsa::iterators_overlap(range.begin() + 5, range.end(), range.begin() + 1));
	}

	SECTION("Overlap if destination range starts inside source range") {
		REQUIRE(dsa::iterators_overlap(range.begin() + 5, range.end(), range.begin() + 6));
	}

	SECTION("No overlap for destination range after source range") {
		REQUIRE(dsa::iterators_overlap(range.begin(), range.begin() + 5, range.begin() + 5));
	}
}

TEST_CASE("Shift memory block onto overlapping partly uninitialized memory", "[algorithms]") {
	std::allocator<int> allocator;

	size_t count = 7;
	auto memory = allocator.allocate(count);

	memory[3] = 1;
	memory[4] = 2;
	memory[5] = 3;

	SECTION("negative count shifs elements to the left") {
		dsa::uninitialized_shift(&memory[3], &memory[5], -2);
		REQUIRE(memory[1] == 1);
		REQUIRE(memory[2] == 2);
		REQUIRE(memory[5] == 3);
	}

	SECTION("positive count shifs elements to the right") {
		dsa::uninitialized_shift(&memory[4], &memory[6], 2);
		REQUIRE(memory[3] == 1);
		REQUIRE(memory[6] == 2);
		REQUIRE(memory[7] == 3);
	}

	allocator.deallocate(memory, count);
}

} // namespace test
