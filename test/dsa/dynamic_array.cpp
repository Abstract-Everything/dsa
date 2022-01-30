#include <dsa/dynamic_array.hpp>

#include <memory>

#include <gtest/gtest.h>

using Int_DArray = dsa::Dynamic_Array<int>;

static_assert(std::is_same_v<Int_DArray::Value, int>);

static_assert(std::is_same_v<decltype(std::declval<Int_DArray>().begin()), int *>);
static_assert(std::is_same_v<decltype(std::declval<const Int_DArray>().begin()), const int *>);
static_assert(std::is_same_v<decltype(std::declval<Int_DArray>().end()), int *>);
static_assert(std::is_same_v<decltype(std::declval<const Int_DArray>().end()), const int *>);

namespace
{

const dsa::Dynamic_Array sample{0, 1, 2};
const dsa::Dynamic_Array sample_long{0, -1, -2, -3, -4, -5};

} // namespace

TEST(dynamic_array, default_initialisation)
{
	dsa::Dynamic_Array<int> array;

	ASSERT_EQ(array.size(), 0ULL);
}

TEST(dynamic_array, sized_initialisation)
{
	constexpr std::size_t   length = 5;
	constexpr int           value  = int{};
	dsa::Dynamic_Array<int> array(length);

	ASSERT_EQ(array.size(), length);
	ASSERT_EQ(array[0], value);
	ASSERT_EQ(array[1], value);
	ASSERT_EQ(array[2], value);
	ASSERT_EQ(array[3], value);
	ASSERT_EQ(array[4], value);
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
	dsa::Dynamic_Array copy(sample);

	ASSERT_EQ(copy, sample);
}

TEST(dynamic_array, move_initialisation)
{
	dsa::Dynamic_Array from(sample);
	dsa::Dynamic_Array to(std::move(from));

	ASSERT_EQ(from.data(), nullptr);
	ASSERT_EQ(to, sample);
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
	copy = sample;

	ASSERT_EQ(copy, sample);
}

TEST(dynamic_array, move_assignment)
{
	dsa::Dynamic_Array      from(sample);
	dsa::Dynamic_Array<int> to;
	to = std::move(from);

	ASSERT_EQ(from.data(), nullptr);
	ASSERT_EQ(to, sample);
}

TEST(dynamic_array, access_operator)
{
	dsa::Dynamic_Array array(sample);

	ASSERT_EQ(array.size(), sample.size());

	*(array.data() + 0ULL) = 2;
	*(array.data() + 1ULL) = 0;
	*(array.data() + 2ULL) = 1;

	ASSERT_EQ(array.size(), sample.size());
	ASSERT_EQ(array[0], 2);
	ASSERT_EQ(array[1], 0);
	ASSERT_EQ(array[2], 1);
}

TEST(dynamic_array, resize_keep_first_few)
{
	dsa::Dynamic_Array array{sample[0], sample[1], sample[2], 3, 4, 5};

	array.resize(sample.size());

	ASSERT_EQ(array, sample);
}

TEST(dynamic_array, resize_default_value)
{
	constexpr int value{};
	dsa::Dynamic_Array
	    expected{sample[0], sample[1], sample[2], value, value, value};

	dsa::Dynamic_Array array(sample);

	array.resize(expected.size());

	ASSERT_EQ(array, expected);
}

TEST(dynamic_array, resize_specified_value)
{
	constexpr int value = -4;
	dsa::Dynamic_Array
	    expected{sample[0], sample[1], sample[2], value, value, value};

	dsa::Dynamic_Array array(sample);

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
	dsa::Dynamic_Array array_1(sample);
	dsa::Dynamic_Array array_2(sample_long);

	swap(array_1, array_2);

	ASSERT_EQ(array_1, sample_long);
	ASSERT_EQ(array_2, sample);
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

TEST(dynamic_array, iterate_empty_array)
{
	dsa::Dynamic_Array<int> empty;
	for ([[maybe_unused]] int value : empty)
	{
		FAIL() << "Expected the body of this loop to not be executed";
	}

	SUCCEED();
}
