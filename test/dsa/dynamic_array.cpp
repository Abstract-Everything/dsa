#include <dsa/dynamic_array.hpp>

#include <gtest/gtest.h>

static_assert(std::is_same_v<dsa::Dynamic_Array<int>::Value, int>);

TEST(dynamic_array, initialisation)
{
	dsa::Dynamic_Array<int> array;

	ASSERT_EQ(array.size(), 0ULL);
}

TEST(dynamic_array, sized_initialisation)
{
	constexpr std::size_t length = 5;
	dsa::Dynamic_Array<int> array(length);

	ASSERT_EQ(array.size(), length);
}
