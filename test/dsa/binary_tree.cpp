#include <dsa/binary_tree.hpp>

#include <gtest/gtest.h>

using Int_Binary_Tree = dsa::Binary_Tree<int>;

static_assert(std::is_same_v<Int_Binary_Tree::Value, int>);

static_assert(std::is_same_v<decltype(*std::declval<Int_Binary_Tree>().begin()), int &>);
static_assert(std::is_same_v<decltype(*std::declval<const Int_Binary_Tree>().begin()), const int &>);
static_assert(std::is_same_v<decltype(*std::declval<Int_Binary_Tree>().end()), int &>);
static_assert(std::is_same_v<decltype(*std::declval<const Int_Binary_Tree>().end()), const int &>);


namespace
{

const dsa::Binary_Tree<int> sample{1, 0, 2};
const dsa::Binary_Tree<int> long_sample{3, 1, 5, 0, 2, 4, 6};

struct Counter
{
	explicit Counter(std::size_t value, std::shared_ptr<int> counter)
	    : m_value(value)
	    , m_counter(std::move(counter))
	{
	}

	[[nodiscard]] friend bool operator<(Counter const &lhs, Counter const &rhs) noexcept
	{
		return lhs.m_value < rhs.m_value;
	}

 private:
	std::size_t          m_value;
	std::shared_ptr<int> m_counter;
};

} // namespace

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

	ASSERT_TRUE(dsa::Binary_Tree<int>::same_structure(copy, sample));

	copy.insert(-1);
	ASSERT_FALSE(sample.contains(-1));
}

TEST(binary_tree, move_initialisation)
{
	dsa::Binary_Tree from(sample);
	dsa::Binary_Tree to(std::move(from));

	ASSERT_TRUE(from.empty());
	ASSERT_TRUE(dsa::Binary_Tree<int>::same_structure(to, sample));
}

TEST(binary_tree, copy_assignment)
{
	dsa::Binary_Tree<int> copy;
	copy = sample;

	ASSERT_TRUE(dsa::Binary_Tree<int>::same_structure(copy, sample));

	copy.insert(-1);
	ASSERT_FALSE(sample.contains(-1));
}

TEST(binary_tree, move_assignment)
{
	dsa::Binary_Tree      from(sample);
	dsa::Binary_Tree<int> to;
	to = std::move(from);

	ASSERT_TRUE(from.empty());
	ASSERT_TRUE(dsa::Binary_Tree<int>::same_structure(to, sample));
}

TEST(binary_tree, swap)
{
	dsa::Binary_Tree binary_tree_1(sample);
	dsa::Binary_Tree binary_tree_2(long_sample);

	swap(binary_tree_1, binary_tree_2);

	ASSERT_TRUE(
	    dsa::Binary_Tree<int>::same_structure(binary_tree_1, long_sample));
	ASSERT_TRUE(dsa::Binary_Tree<int>::same_structure(binary_tree_2, sample));
}

TEST(binary_tree, insert_single_value)
{
	dsa::Binary_Tree<int> binary_tree;

	binary_tree.insert(1);

	ASSERT_FALSE(binary_tree.empty());
	ASSERT_FALSE(binary_tree.contains(0));
	ASSERT_TRUE(binary_tree.contains(1));
	ASSERT_FALSE(binary_tree.contains(2));
}

TEST(binary_tree, insert_multiple_values)
{
	dsa::Binary_Tree<int> binary_tree;

	binary_tree.insert(0);
	binary_tree.insert(1);
	binary_tree.insert(2);

	ASSERT_FALSE(binary_tree.empty());
	ASSERT_TRUE(binary_tree.contains(0));
	ASSERT_TRUE(binary_tree.contains(1));
	ASSERT_TRUE(binary_tree.contains(2));
}

TEST(binary_tree, erase_head)
{
	dsa::Binary_Tree<int>       binary_tree{0};
	const dsa::Binary_Tree<int> expected;

	binary_tree.erase(0);

	ASSERT_TRUE(binary_tree.empty());
	ASSERT_TRUE(dsa::Binary_Tree<int>::same_structure(binary_tree, expected));
}

TEST(binary_tree, erase_leaf)
{
	dsa::Binary_Tree<int>       binary_tree{0, 1};
	const dsa::Binary_Tree<int> expected{0};

	binary_tree.erase(1);

	ASSERT_TRUE(dsa::Binary_Tree<int>::same_structure(binary_tree, expected));
}

TEST(binary_tree, erase_node_no_left_child)
{
	dsa::Binary_Tree<int>       binary_tree{0, 1, 2};
	const dsa::Binary_Tree<int> expected{0, 2};

	binary_tree.erase(1);

	ASSERT_TRUE(dsa::Binary_Tree<int>::same_structure(binary_tree, expected));
}

TEST(binary_tree, erase_node_immediate_left_child_present)
{
	dsa::Binary_Tree<int>       binary_tree{0, 3, 4, 2, 1};
	const dsa::Binary_Tree<int> expected{0, 2, 4, 1};

	binary_tree.erase(3);

	ASSERT_TRUE(dsa::Binary_Tree<int>::same_structure(binary_tree, expected));
}

TEST(binary_tree, erase_node_deep_left_child_present)
{
	dsa::Binary_Tree<int>       binary_tree{3, 1, 0, 2};
	const dsa::Binary_Tree<int> expected{2, 1, 0};

	binary_tree.erase(3);

	ASSERT_TRUE(dsa::Binary_Tree<int>::same_structure(binary_tree, expected));
}

TEST(binary_tree, clear)
{
	dsa::Binary_Tree binary_tree(sample);

	binary_tree.clear();

	ASSERT_TRUE(binary_tree.empty());
}

TEST(binary_tree, comparison_operator_empty)
{
	dsa::Binary_Tree<int> binary_tree_1;
	dsa::Binary_Tree<int> binary_tree_2;

	ASSERT_EQ(binary_tree_1, binary_tree_2);
}

TEST(binary_tree, comparison_operator_differing_element)
{
	dsa::Binary_Tree binary_tree_1{0};
	dsa::Binary_Tree binary_tree_2{1};

	ASSERT_NE(binary_tree_1, binary_tree_2);
}

TEST(binary_tree, comparison_operator_same_single_element)
{
	dsa::Binary_Tree<int> binary_tree_1{0};
	dsa::Binary_Tree<int> binary_tree_2{0};

	ASSERT_EQ(binary_tree_1, binary_tree_2);
}

TEST(binary_tree, comparison_operator_same_multiple_elements)
{
	dsa::Binary_Tree<int> binary_tree_1{0, 1, 2};
	dsa::Binary_Tree<int> binary_tree_2{0, 1, 2};

	ASSERT_EQ(binary_tree_1, binary_tree_2);
}

TEST(binary_tree, comparison_operator_single_differing_element)
{
	dsa::Binary_Tree<int> binary_tree_1{0, 1, 2};
	dsa::Binary_Tree<int> binary_tree_2{0, 1, 3};

	ASSERT_NE(binary_tree_1, binary_tree_2);
}

TEST(binary_tree, comparison_operator_differing_size)
{
	dsa::Binary_Tree<int> binary_tree_1{0, 1};
	dsa::Binary_Tree<int> binary_tree_2{0, 1, 2};

	ASSERT_NE(binary_tree_1, binary_tree_2);
}

TEST(binary_tree, comparison_operator_differing_insert_order)
{
	dsa::Binary_Tree<int> binary_tree_1{0, 1, 2};
	dsa::Binary_Tree<int> binary_tree_2{0, 2, 1};

	ASSERT_EQ(binary_tree_1, binary_tree_2);
}

TEST(binary_tree, destroy_elements)
{
	constexpr std::size_t count   = 3;
	std::shared_ptr<int>  counter = std::make_shared<int>(0);
	{
		dsa::Binary_Tree<Counter> binary_tree;
		for (std::size_t i = 0; i < count; ++i)
		{
			binary_tree.insert(Counter(i, counter));
		}

		ASSERT_EQ(counter.use_count(), count + 1);
	}

	ASSERT_EQ(counter.use_count(), 1);
}

TEST(binary_tree, iterate_empty_list)
{
	dsa::Binary_Tree<int> empty;
	for ([[maybe_unused]] int value : empty)
	{
		FAIL() << "Expected the body of this loop to not be executed";
	}

	SUCCEED();
}

