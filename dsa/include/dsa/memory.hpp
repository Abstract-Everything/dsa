#ifndef DSA_MEMORY_HPP
#define DSA_MEMORY_HPP

#include <algorithm>
#include <cassert>
#include <iterator>
#include <memory>

namespace dsa
{

namespace detail
{

template<class Iterator>
void overlapping_uninitialized_move(Iterator begin, Iterator end, Iterator destination) {
	while (begin != end)
	{
		std::uninitialized_move_n(begin++, 1, destination++);
	}
}

} // namespace detail

/**
 * @brief Checks Whether two iterator ranges overlap
 */
template<class Iterator>
bool iterators_overlap(Iterator begin, Iterator end, Iterator destination) {
	Iterator iter = begin;
	while (iter++ != end)
	{
		if (destination == begin || destination == end)
		{
			return true;
		}
		destination++;
	}
	return false;
}

/**
 * @brief shifts [begin, end) into [begin + 1, end + 1). Where end points to
 * uninitialized memory.
 */
template<class Iterator>
void uninitialized_shift(Iterator begin, Iterator end, int count = 1) {
	assert(count != 0 && "Elements must be moved into uninitialized memory");
	if (count > 0)
	{
		detail::overlapping_uninitialized_move(
		    std::reverse_iterator(end),
		    std::reverse_iterator(begin),
		    std::reverse_iterator(end + count));
	}
	else
	{
		detail::overlapping_uninitialized_move(begin, end, begin + count);
	}
}

/**
 * @brief Move-construct from the range [begin,end) into destination.
 *
 * Note: std::uninitialized_move is undefined if the source and destination
 * ranges overlap, use uninitialized_shift in such cases.
 */
template<class Iterator>
void uninitialized_move(Iterator begin, Iterator end, Iterator destination) {
	assert(!iterators_overlap(begin, end, destination) && "std::uninitialized_move is undefined if the source and destination ranges overlap");
	std::uninitialized_move(begin, end, destination);
}

} // namespace dsa

#endif
