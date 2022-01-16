#include <dsa/binary_tree.hpp>

#include <gtest/gtest.h>

static_assert(std::is_same_v<dsa::Binary_Tree<int>::Value, int>);

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
