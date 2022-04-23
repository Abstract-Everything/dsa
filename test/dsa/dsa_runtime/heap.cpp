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
const dsa::Heap<int> sample_long{10, 11, 12, 13, 14, 15};

} // namespace

static_assert(std::is_same_v<dsa::Heap<int>::Value, int>);

TEST(heap, default_initialisation)
{
	dsa::Heap<int> heap;

	ASSERT_TRUE(heap.empty());
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

TEST(heap, move_assignment)
{
	dsa::Heap      from(sample);
	dsa::Heap<int> to;
	to = std::move(from);

	ASSERT_EQ(to.size(), 3ULL);
	ASSERT_EQ(to.top(), 0);
	ASSERT_THAT(to, is_heap());
}

TEST(heap, swap)
{
	dsa::Heap heap1(sample);
	dsa::Heap heap2(sample_long);

	ASSERT_EQ(heap1.size(), 3ULL);
	ASSERT_EQ(heap1.top(), 0);

	ASSERT_EQ(heap2.size(), 6ULL);
	ASSERT_EQ(heap2.top(), 10);

	swap(heap1, heap2);

	ASSERT_EQ(heap1.size(), 6ULL);
	ASSERT_EQ(heap1.top(), 10);

	ASSERT_EQ(heap2.size(), 3ULL);
	ASSERT_EQ(heap2.top(), 0);

	ASSERT_THAT(heap1, is_heap());
	ASSERT_THAT(heap2, is_heap());
}

TEST(heap, push_single_value)
{
	dsa::Heap<int> heap;
	heap.push(0);

	ASSERT_FALSE(heap.empty());
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

TEST(heap, pop_single_value)
{
	dsa::Heap heap{0};
	heap.pop();

	ASSERT_TRUE(heap.empty());
	ASSERT_THAT(heap, is_heap());
}

TEST(heap, pop_value_replaced)
{
	dsa::Heap heap{0, 1};
	heap.pop();

	ASSERT_EQ(heap.size(), 1ULL);
	ASSERT_EQ(heap.top(), 1);
	ASSERT_THAT(heap, is_heap());
}

TEST(heap, pop_value_replaced_by_left_child)
{
	dsa::Heap heap{0, 1, 2, 3};
	heap.pop();

	ASSERT_EQ(heap.size(), 3ULL);
	ASSERT_EQ(heap.top(), 1);
	ASSERT_THAT(heap, is_heap());
}

TEST(heap, pop_value_replaced_by_right_child)
{
	dsa::Heap heap{0, 2, 1, 3};
	heap.pop();

	ASSERT_EQ(heap.size(), 3ULL);
	ASSERT_EQ(heap.top(), 1);
	ASSERT_THAT(heap, is_heap());
}

TEST(heap, pop_handle_node_single_child)
{
	dsa::Heap heap{0, 2, 1, 3, 5, 7, 9};
	heap.pop();

	ASSERT_EQ(heap.size(), 6ULL);
	ASSERT_EQ(heap.top(), 1);
	ASSERT_THAT(heap, is_heap());
}

TEST(heap, pop_exchanged_element_smaller_than_deepest_nodes)
{
	dsa::Heap heap{0, 1, 2, 6, 4, 5, 3};
	heap.pop();

	ASSERT_EQ(heap.size(), 6ULL);
	ASSERT_EQ(heap.top(), 1);
	ASSERT_THAT(heap, is_heap());
}
