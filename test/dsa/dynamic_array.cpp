#include <dsa/dynamic_array.hpp>

#include <gtest/gtest.h>

static_assert(std::is_same_v<dsa::Dynamic_Array<int>::Value, int>);

namespace
{

const dsa::Dynamic_Array test_array{0, 1, 2};

}

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

TEST(dynamic_array, copy_initialisation)
{
	dsa::Dynamic_Array copy(test_array);

	ASSERT_EQ(copy, test_array);
}

TEST(dynamic_array, move_initialisation)
{
	dsa::Dynamic_Array from(test_array);
	dsa::Dynamic_Array to(std::move(from));

	ASSERT_EQ(from.data(), nullptr);
	ASSERT_EQ(to, test_array);
}

TEST(dynamic_array, comparison_operator_differing_size)
{
	dsa::Dynamic_Array<int> array_1(1);
	dsa::Dynamic_Array<int> array_2(2);

	ASSERT_NE(array_1, array_2);
}

TEST(dynamic_array, comparison_operator_differing_element)
{
	dsa::Dynamic_Array array_1{0, 0, 0};
	dsa::Dynamic_Array array_2{0, 0, 1};
	dsa::Dynamic_Array array_3{1, 0, 1};

	ASSERT_NE(array_1, array_2);
	ASSERT_NE(array_2, array_3);
}

TEST(dynamic_array, comparison_operator_equal)
{
	dsa::Dynamic_Array array_1{0, -1, 5};
	dsa::Dynamic_Array array_2{0, -1, 5};

	ASSERT_EQ(array_1, array_2);
}

TEST(dynamic_array, copy_assignment)
{
	dsa::Dynamic_Array<int> copy;
	copy = test_array;

	ASSERT_EQ(copy, test_array);
}

TEST(dynamic_array, move_assignment)
{
	dsa::Dynamic_Array      from(test_array);
	dsa::Dynamic_Array<int> to;
	to = std::move(from);

	ASSERT_EQ(from.data(), nullptr);
	ASSERT_EQ(to, test_array);
}

TEST(dynamic_array, access_operator)
{
	dsa::Dynamic_Array array(test_array);

	ASSERT_EQ(array.size(), test_array.size());
	ASSERT_EQ(array[0], 0);
	ASSERT_EQ(array[1], 1);
	ASSERT_EQ(array[2], 2);

	array[0] = 2;
	array[1] = 0;
	array[2] = 1;

	ASSERT_EQ(array.size(), test_array.size());
	ASSERT_EQ(array[0], 2);
	ASSERT_EQ(array[1], 0);
	ASSERT_EQ(array[2], 1);
}
