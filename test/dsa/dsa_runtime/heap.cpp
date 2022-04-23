#include <dsa/algorithms.hpp>
#include <dsa/default_allocator.hpp>
#include <dsa/heap.hpp>

#include <fmt/format.h>

#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace testing;

namespace
{

MATCHER(is_heap, fmt::format("to {}be a min heap", negation ? "not " : ""))
{
	return dsa::is_heap(arg.storage().begin(), arg.storage().end(), std::less{});
}

const dsa::Heap<int> sample{0, 1, 2};

} // namespace

static_assert(std::is_same_v<dsa::Heap<int>::Value, int>);

TEST(heap, default_initialisation)
{
	dsa::Heap<int> heap;

	ASSERT_EQ(heap.size(), 0ULL);
	ASSERT_THAT(heap, is_heap());
}

TEST(heap, list_initialisation)
{
	dsa::Heap heap{0, 1, 2};

	ASSERT_EQ(heap.size(), 3ULL);
	ASSERT_EQ(heap.top(), 0);
	ASSERT_THAT(heap, is_heap());
}

TEST(heap, copy_initialisation)
{
	dsa::Heap heap(sample);

	ASSERT_EQ(heap.size(), 3ULL);
	ASSERT_EQ(heap.top(), 0);
	ASSERT_THAT(heap, is_heap());
}

TEST(heap, move_initialisation)
{
	dsa::Heap from(sample);
	dsa::Heap to(std::move(from));

	ASSERT_EQ(to.size(), 3ULL);
	ASSERT_EQ(to.top(), 0);
	ASSERT_THAT(to, is_heap());
}

TEST(heap, copy_assignment)
{
	dsa::Heap<int> heap;
	heap = sample;

	ASSERT_EQ(heap.size(), 3ULL);
	ASSERT_EQ(heap.top(), 0);
	ASSERT_THAT(heap, is_heap());
}

TEST(heap, push_single_value)
{
	dsa::Heap<int> heap;
	heap.push(0);

	ASSERT_EQ(heap.size(), 1ULL);
	ASSERT_EQ(heap.top(), 0);
	ASSERT_THAT(heap, is_heap());
}

TEST(heap, push_value_is_in_order)
{
	dsa::Heap<int> heap;
	heap.push(1);
	heap.push(3);
	heap.push(5);
	heap.push(7);

	ASSERT_EQ(heap.size(), 4ULL);
	ASSERT_EQ(heap.top(), 1);
	ASSERT_THAT(heap, is_heap());
}

TEST(heap, push_value_swaps_parent)
{
	dsa::Heap<int> heap;
	heap.push(1);
	heap.push(3);
	heap.push(5);
	heap.push(2);

	ASSERT_EQ(heap.size(), 4ULL);
	ASSERT_EQ(heap.top(), 1);
	ASSERT_THAT(heap, is_heap());
}

TEST(heap, push_value_propogated_to_root)
{
	dsa::Heap<int> heap;
	heap.push(1);
	heap.push(2);
	heap.push(3);
	heap.push(4);
	heap.push(5);
	heap.push(6);
	heap.push(0);

	ASSERT_EQ(heap.size(), 7ULL);
	ASSERT_EQ(heap.top(), 0);
	ASSERT_THAT(heap, is_heap());
}
