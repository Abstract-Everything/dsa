#include <dsa/dynamic_array.hpp>

#include <memory>

#include <gtest/gtest.h>

static_assert(std::is_same_v<dsa::Dynamic_Array<int>::Value, int>);

namespace
{

const dsa::Dynamic_Array test_array_1{0, 1, 2};
const dsa::Dynamic_Array test_array_2{0, -1, -2, -3, -4, -5};

} // namespace

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

TEST(dynamic_array, sized_initialisation_default_value)
{
	constexpr std::size_t length = 3;
	constexpr int         value  = -5;
	dsa::Dynamic_Array    array(length, value);

	ASSERT_EQ(array.size(), length);
	ASSERT_EQ(array[0], value);
	ASSERT_EQ(array[1], value);
	ASSERT_EQ(array[2], value);
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
	dsa::Dynamic_Array copy(test_array_1);

	ASSERT_EQ(copy, test_array_1);
}

TEST(dynamic_array, move_initialisation)
{
	dsa::Dynamic_Array from(test_array_1);
	dsa::Dynamic_Array to(std::move(from));

	ASSERT_EQ(from.data(), nullptr);
	ASSERT_EQ(to, test_array_1);
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
	copy = test_array_1;

	ASSERT_EQ(copy, test_array_1);
}

TEST(dynamic_array, move_assignment)
{
	dsa::Dynamic_Array      from(test_array_1);
	dsa::Dynamic_Array<int> to;
	to = std::move(from);

	ASSERT_EQ(from.data(), nullptr);
	ASSERT_EQ(to, test_array_1);
}

TEST(dynamic_array, access_operator)
{
	dsa::Dynamic_Array array(test_array_1);

	ASSERT_EQ(array.size(), test_array_1.size());
	ASSERT_EQ(array[0], 0);
	ASSERT_EQ(array[1], 1);
	ASSERT_EQ(array[2], 2);

	array[0] = 2;
	array[1] = 0;
	array[2] = 1;

	ASSERT_EQ(array.size(), test_array_1.size());
	ASSERT_EQ(array[0], 2);
	ASSERT_EQ(array[1], 0);
	ASSERT_EQ(array[2], 1);
}

TEST(dynamic_array, resize_keep_first_few)
{
	auto const        &sample = test_array_1;
	dsa::Dynamic_Array array{sample[0], sample[1], sample[2], 3, 4, 5};

	array.resize(sample.size());

	ASSERT_EQ(array, sample);
}

TEST(dynamic_array, resize_default_value)
{
	constexpr int      value  = -4;
	auto const        &sample = test_array_1;
	dsa::Dynamic_Array expected{
	    test_array_1[0],
	    test_array_1[1],
	    test_array_1[2],
	    value,
	    value,
	    value};

	dsa::Dynamic_Array array(test_array_1);

	array.resize(expected.size(), value);

	ASSERT_EQ(array, expected);
}

TEST(dynamic_array, resize_data_should_not_be_nullptr)
{
	dsa::Dynamic_Array array{0, 1, 2, 4, 5, 6};

	array.resize(0);
	ASSERT_EQ(array.size(), 0ULL);
}

TEST(dynamic_array, swap)
{
	dsa::Dynamic_Array array_1(test_array_1);
	dsa::Dynamic_Array array_2(test_array_2);

	swap(array_1, array_2);

	ASSERT_EQ(array_1, test_array_2);
	ASSERT_EQ(array_2, test_array_1);
}

TEST(dynamic_array, destroy_elements)
{
	constexpr std::size_t length  = 3;
	std::shared_ptr<int>  counter = std::make_shared<int>(0);
	{
		dsa::Dynamic_Array<std::shared_ptr<int>> array(length, counter);
		ASSERT_EQ(counter.use_count(), length + 1);
	}

	ASSERT_EQ(counter.use_count(), 1);
}
