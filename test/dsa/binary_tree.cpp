#include <dsa/binary_tree.hpp>

#include <gtest/gtest.h>

static_assert(std::is_same_v<dsa::Binary_Tree<int>::Value, int>);

namespace
{

const dsa::Binary_Tree<int> sample{1, 0, 2};
const dsa::Binary_Tree<int> long_sample{3, 1, 5, 0, 2, 4, 6};

}

TEST(binary_tree, default_initialisation)
{
	dsa::Binary_Tree<int> binary_tree;

	ASSERT_TRUE(binary_tree.empty());
	ASSERT_EQ(binary_tree.size(), 0ULL);
}

TEST(binary_tree, list_initialisation)
{
	dsa::Binary_Tree<int> binary_tree{0, 1, 2};

	ASSERT_FALSE(binary_tree.empty());
	ASSERT_TRUE(binary_tree.contains(0));
	ASSERT_TRUE(binary_tree.contains(1));
	ASSERT_TRUE(binary_tree.contains(2));
}

TEST(binary_tree, copy_initialisation)
{
	dsa::Binary_Tree copy(sample);

	ASSERT_EQ(copy, sample);

	copy.insert(-1);
	ASSERT_FALSE(sample.contains(-1));
}

TEST(binary_tree, move_initialisation)
{
	dsa::Binary_Tree from(sample);
	dsa::Binary_Tree to(std::move(from));

	ASSERT_TRUE(from.empty());
	ASSERT_EQ(to, sample);
}

TEST(binary_tree, copy_assignment)
{
	dsa::Binary_Tree<int> copy;
	copy = sample;

	ASSERT_EQ(copy, sample);

	copy.insert(-1);
	ASSERT_FALSE(sample.contains(-1));
}

TEST(binary_tree, move_assignment)
{
	dsa::Binary_Tree      from(sample);
	dsa::Binary_Tree<int> to;
	to = std::move(from);

	ASSERT_TRUE(from.empty());
	ASSERT_EQ(to, sample);
}

TEST(binary_tree, swap)
{
	dsa::Binary_Tree binary_tree_1(sample);
	dsa::Binary_Tree binary_tree_2(long_sample);

	swap(binary_tree_1, binary_tree_2);

	ASSERT_EQ(binary_tree_1, long_sample);
	ASSERT_EQ(binary_tree_2, sample);
}
