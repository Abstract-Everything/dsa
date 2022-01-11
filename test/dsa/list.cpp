#include <dsa/list.hpp>

#include <gtest/gtest.h>

static_assert(std::is_same_v<dsa::List<int>::Value, int>);

TEST(list, default_initialisation)
{
	dsa::List<int> list;

	ASSERT_TRUE(list.empty());
	ASSERT_EQ(list.size(), 0ULL);
}
