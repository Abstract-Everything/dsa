#include <dsa/list.hpp>

#include <gtest/gtest.h>

static_assert(std::is_same_v<dsa::List<int>::Value, int>);

TEST(list, default_initialisation)
{
	dsa::List<int> list;

	ASSERT_TRUE(list.empty());
	ASSERT_EQ(list.size(), 0ULL);
}

TEST(list, list_initialisation)
{
	constexpr std::size_t length = 3;
	dsa::List             list{0, 1, 2};

	ASSERT_FALSE(list.empty());
	ASSERT_EQ(list.size(), length);
	ASSERT_EQ(list.front(), 0);
	ASSERT_EQ(list[0], 0);
	ASSERT_EQ(list[1], 1);
	ASSERT_EQ(list[2], 2);
}
