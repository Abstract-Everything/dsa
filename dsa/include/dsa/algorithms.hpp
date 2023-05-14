#ifndef DSA_ALGORITHMS_HPP
#define DSA_ALGORITHMS_HPP

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
 *  @brief Uses linear search to find an element that satisfies the condition in
 *  the given range
 *  @return An empty std::optional if no element is found, otherwise it contains
 *  the iterator of the value in the range
 */
template<typename Iterator, typename Traits = std::iterator_traits<Iterator>>
auto linear_search(Iterator begin, Iterator end, auto const& predicate)
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

} // namespace dsa

#endif
