#include <dsa/algorithms.hpp>
#include <dsa/vector.hpp>

#include <utility>

#include <gtest/gtest.h>

TEST(is_heap, empty)
{
	dsa::Vector<int> empty;

	ASSERT_TRUE(dsa::is_heap(empty.begin(), empty.end(), std::less{}));
}

TEST(is_heap, just_root)
{
	dsa::Vector<int> root{1};

	ASSERT_TRUE(dsa::is_heap(root.begin(), root.end(), std::less{}));
}

TEST(is_heap, root_children)
{
	dsa::Vector<int> vector{0, 1, 2};

	ASSERT_TRUE(dsa::is_heap(vector.begin(), vector.end(), std::less{}));
}

TEST(is_heap, children_invalid)
{
	dsa::Vector<int> vector{2, 1, 2};

	ASSERT_FALSE(dsa::is_heap(vector.begin(), vector.end(), std::less{}));
}

TEST(is_heap, large_valid)
{
	dsa::Vector<int> vector{0, 10, 1000, 20, 30, 2000, 3000, 25, 30, 33, 32};

	ASSERT_TRUE(dsa::is_heap(vector.begin(), vector.end(), std::less{}));
}

TEST(is_heap, large_invalid)
{
	dsa::Vector<int> vector{0, 10, 1000, 20, 30, 2000, 3000, 25, 30, 33, 32, 1999};

	ASSERT_FALSE(dsa::is_heap(vector.begin(), vector.end(), std::less{}));
}

TEST(is_heap, different_comparator)
{
	dsa::Vector<int> vector{2, 1, 2};

	ASSERT_TRUE(dsa::is_heap(vector.begin(), vector.end(), std::greater{}));
}
