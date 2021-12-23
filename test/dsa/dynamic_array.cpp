#include <dsa/dynamic_array.hpp>

#include <gtest/gtest.h>

static_assert(std::is_same_v<dsa::Dynamic_Array<int>::Value, int>);

TEST(dynamic_array, default_initialisation)
{
	dsa::Dynamic_Array<int> array;

	ASSERT_EQ(array.size(), 0ULL);
}

TEST(dynamic_array, sized_initialisation)
{
	constexpr std::size_t   length = 5;
	dsa::Dynamic_Array<int> array(length);

	ASSERT_EQ(array.size(), length);
}

TEST(dynamic_array, list_initialisation)
{
	constexpr std::size_t length = 3;
	dsa::Dynamic_Array    array{0, 1, 2};

	ASSERT_EQ(array.size(), length);
	ASSERT_EQ(array[0], 0);
	ASSERT_EQ(array[1], 1);
	ASSERT_EQ(array[2], 2);
}
