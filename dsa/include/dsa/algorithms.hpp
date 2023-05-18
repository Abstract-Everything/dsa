#ifndef DSA_ALGORITHMS_HPP
#define DSA_ALGORITHMS_HPP

#include <dsa/vector.hpp>

#include <functional>
#include <optional>
#include <utility>

namespace dsa
{

/**
 * @brief Checks if the given range forms a heap
 *
 * An array is a heap if every element and its corresponding children return
 * true when compared using comparator(parent, child) or parent == child. The
 * child of a parent is equal to 2 * parent_index + 1.
 *
 * Example, the following array relationships are as follows:
 * Index: 0 1 2 3 4 5 6
 * Value: a b c d e f g
 *
 * Relationships:
 *         a
 *     b       c
 *   d   e   f   g
 *
 * So for example the input:
 *   begin,          end, operator<
 *     0, 1, 2, 1, 1, 0
 *
 * is not a heap because the third element (2) < its left child (0) is false
 *
 * Equal elements are valid because without this property a weird behaviour is
 * introduced, consider the following: array: 0, 1, 1
 *
 * This clearly forms a min heap because 0 < 1. If we pop the top element we end
 * up with 1, 1 which does not form a heap if equality is not allowed. Thus this
 * results in popping the top element invalidating the heap
 */
template<typename Iterator, typename Comparator>
bool is_heap(Iterator begin, Iterator end, Comparator const &comparator) {
	bool increment_parent = false;
	for (Iterator parent = begin, child = begin + 1; child < end; ++child)
	{
		if (*parent != *child && !comparator(*parent, *child))
		{
			return false;
		}

		if (increment_parent)
		{
			++parent;
		}

		increment_parent = !increment_parent;
	}
	return true;
}

/**
 *  @brief Checks if every pair in the given range satisfies
 *  comparator(first, second)
 */
template<typename Iterator>
bool is_sorted(Iterator begin, Iterator end, auto const &comparator) {
	if (begin == end)
	{
		return true;
	}

	for (Iterator i = begin, j = begin + 1; j != end; ++i, ++j)
	{
		if (!comparator(*i, *j))
		{
			return false;
		}
	}
	return true;
}

/**
 *  @brief Checks if the given range is sorted in ascending order
 */
template<typename Iterator>
bool is_sorted(Iterator begin, Iterator end) {
	return dsa::is_sorted(begin, end, std::less{});
}

/**
 *  @brief Uses insertion sort on the given range such that each pair satisfies
 *  comparator(first, second)
 */
template<typename Iterator>
void insertion_sort(Iterator begin, Iterator end, auto const &comparator) {
	using std::swap;

	if (begin == end)
	{
		return;
	}

	for (auto i = begin + 1; i != end; ++i)
	{
		for (auto j = i; j != begin && !comparator(*(j - 1), *j); --j)
		{
			swap(*(j - 1), *j);
		}
	}
}

/**
 *  @brief Uses insertion sort to sort the given range in ascending order
 */
template<typename Iterator>
void insertion_sort(Iterator begin, Iterator end) {
	return insertion_sort(begin, end, std::less{});
}

/**
 *  @brief Uses selection sort on the given range such that each pair satisfies
 *  comparator(first, second)
 */
template<typename Iterator>
void selection_sort(Iterator begin, Iterator end, auto const &comparator) {
	using std::swap;

	if (begin == end)
	{
		return;
	}

	// We do not need to find the smallest element when only one element is
	// left to be sorted
	auto const last = end - 1;
	for (auto i = begin; i != last; ++i)
	{
		Iterator smallest = i;
		for (auto j = i + 1; j != end; ++j)
		{
			if (!comparator(*smallest, *j))
			{
				smallest = j;
			}
		}
		swap(*smallest, *i);
	}
}

/**
 *  @brief Uses selection sort to sort the given range in ascending order
 */
template<typename Iterator>
void selection_sort(Iterator begin, Iterator end) {
	return selection_sort(begin, end, std::less{});
}

/**
 *  @brief Uses merge sort on the given range such that each pair satisfies
 *  comparator(first, second)
 */
template<
    typename Iterator,
    typename Allocator =
	Default_Allocator<typename std::iterator_traits<Iterator>::value_type>>
void merge_sort(Iterator begin, Iterator end, auto const &comparator) {
	using Traits = std::iterator_traits<Iterator>;
	using std::swap;

	auto const difference = end - begin;
	if (difference < 2)
	{
		return;
	}

	if (difference == 2)
	{
		if (!comparator(*begin, *(begin + 1)))
		{
			swap(*begin, *(begin + 1));
		}
		return;
	}

	auto const     half   = difference / 2;
	Iterator const middle = begin + half;

	// TODO: implement inplace merge sort
	dsa::Vector<typename Traits::value_type, Allocator> first_half;
	first_half.reserve(static_cast<size_t>(half));
	for (auto i = begin; i != middle; ++i)
	{
		first_half.append(std::move(*i));
	}

	merge_sort<Iterator, Allocator>(
	    first_half.begin(),
	    first_half.end(),
	    comparator);
	merge_sort<Iterator, Allocator>(middle, end, comparator);

	auto     first  = first_half.begin();
	Iterator second = middle;
	for (Iterator i = begin; i != end; ++i)
	{
		if (second == end
		    || (first != first_half.end() && comparator(*first, *second)))
		{
			swap(*i, *first++);
		}
		else
		{
			swap(*i, *second++);
		}
	}
}

/**
 *  @brief Uses merge sort to sort the given range in ascending order
 */
template<
    typename Iterator,
    typename Allocator =
	Default_Allocator<typename std::iterator_traits<Iterator>::value_type>>
void merge_sort(Iterator begin, Iterator end) {
	return merge_sort<Iterator, Allocator>(begin, end, std::less{});
}

/**
 *  @brief Uses linear search to find an element in the given range
 *  @return An empty std::optional if no element is found, otherwise it contains
 *  the iterator of the value in the range
 */
template<typename Iterator, typename Traits = std::iterator_traits<Iterator>>
auto linear_search(Iterator begin, Iterator end, auto const &predicate)
    -> std::optional<Iterator> {
	for (auto i = begin; i != end; ++i)
	{
		if (std::is_eq(predicate(*i)))
		{
			return i;
		}
	}
	return {};
}

/**
 *  @brief Uses linear search to find an element in the given range
 *  @return An empty std::optional if no element is found, otherwise it contains
 *  the iterator of the value in the range
 */
template<typename Iterator, typename Traits = std::iterator_traits<Iterator>>
auto linear_search(Iterator begin, Iterator end, typename Traits::value_type const &value)
    -> std::optional<Iterator> {
	return linear_search(begin, end, [&](typename Traits::value_type const &other) {
		return other <=> value;
	});
}

/**
 *  @brief Uses binary search to find an element in the given range
 *  @return An empty std::optional if no element is found, otherwise it contains
 *  the iterator of the value in the range
 */
template<typename Iterator, typename Traits = std::iterator_traits<Iterator>>
auto binary_search(Iterator begin, Iterator end, auto const &predicate)
    -> std::optional<Iterator> {
	while (begin != end)
	{
		// We want to round down so that we stay in the begin, end - 1
		// range
		auto                 distance = (end - begin) & (~1U);
		Iterator             mid      = begin + (distance / 2);
		std::strong_ordering order    = predicate(*mid);
		if (std::is_eq(order))
		{
			return mid;
		}

		if (std::is_gt(order))
		{
			end = mid;
		}
		else
		{
			begin = mid + 1;
		}
	}
	return {};
}

/**
 *  @brief Uses binary search to find an element in the given range
 *  @return An empty std::optional if no element is found, otherwise it contains
 *  the iterator of the value in the range
 */
template<typename Iterator, typename Traits = std::iterator_traits<Iterator>>
auto binary_search(Iterator begin, Iterator end, typename Traits::value_type const &value)
    -> std::optional<Iterator> {
	return binary_search(begin, end, [&](typename Traits::value_type const &other) {
		return other <=> value;
	});
}

/**
 *  @brief Uses sort and binary search in order to find two elements that add up
 *  the the given sum
 *  @return An empty std::optional if no such elements are found, otherwise it
 *  contains a pair of two iterators whose sum add up to the given value
 *
 *  TODO: This can be done with a Hashmap, but since we did not implement it
 *  yet we use the sort + binary search version
 */
template<typename Iterator, typename Traits = std::iterator_traits<Iterator>>
auto sum_components_search(
    Iterator                           begin,
    Iterator                           end,
    typename Traits::value_type const &value)
    -> std::optional<std::pair<Iterator, Iterator>> {
	dsa::Vector<Iterator> sorted_iterators;
	sorted_iterators.reserve(static_cast<size_t>(end - begin));
	for (auto i = begin; i != end; ++i)
	{
		sorted_iterators.append(i);
	}
	merge_sort(
	    sorted_iterators.begin(),
	    sorted_iterators.end(),
	    [](Iterator const &lhs, Iterator const &rhs) { return *lhs < *rhs; });

	for (auto i = sorted_iterators.begin(); i != sorted_iterators.end(); ++i)
	{
		auto complement = binary_search(
		    i + 1,
		    sorted_iterators.end(),
		    [&](Iterator const &it) { return **i + *it <=> value; });
		if (complement.has_value())
		{
			return std::pair{*i, *complement.value()};
		}
	}
	return {};
}

} // namespace dsa

#endif
