#include <dsa/vector.hpp>

#include <gtest/gtest.h>

using Int_Vector = dsa::Vector<int>;

static_assert(std::is_same_v<Int_Vector::Value, int>);

static_assert(std::is_same_v<decltype(std::declval<Int_Vector>().begin()), int *>);
static_assert(std::is_same_v<decltype(std::declval<const Int_Vector>().begin()), const int *>);
static_assert(std::is_same_v<decltype(std::declval<Int_Vector>().end()), int *>);
static_assert(std::is_same_v<decltype(std::declval<const Int_Vector>().end()), const int *>);

namespace
{

const dsa::Vector<int> sample{0, 1, 2};

}

TEST(vector, default_initialisation)
{
	dsa::Vector<int> vector;

	ASSERT_TRUE(vector.empty());
	ASSERT_EQ(vector.capacity(), 0ULL);
}

TEST(vector, sized_initialisation)
{
	constexpr std::size_t length = 5;
	constexpr int         value  = int{};
	dsa::Vector<int>      vector(length);

	ASSERT_FALSE(vector.empty());
	ASSERT_EQ(vector.size(), length);
	ASSERT_EQ(vector.capacity(), length);
	ASSERT_EQ(vector[0], value);
	ASSERT_EQ(vector[1], value);
	ASSERT_EQ(vector[2], value);
	ASSERT_EQ(vector[3], value);
	ASSERT_EQ(vector[4], value);
}

TEST(vector, sized_initialisation_default_value)
{
	constexpr std::size_t length = 3;
	constexpr int         value  = -5;
	dsa::Vector           vector(length, value);

	ASSERT_FALSE(vector.empty());
	ASSERT_EQ(vector.size(), length);
	ASSERT_EQ(vector.capacity(), length);
	ASSERT_EQ(vector[0], value);
	ASSERT_EQ(vector[1], value);
	ASSERT_EQ(vector[2], value);
}

TEST(vector, list_initialisation)
{
	constexpr std::size_t length = 3;
	dsa::Vector           vector{0, 1, 2};

	ASSERT_FALSE(vector.empty());
	ASSERT_EQ(vector.size(), length);
	ASSERT_EQ(vector.capacity(), length);
	ASSERT_EQ(vector[0], 0);
	ASSERT_EQ(vector[1], 1);
	ASSERT_EQ(vector[2], 2);
}

TEST(vector, copy_initialisation)
{
	dsa::Vector copy(sample);

	ASSERT_EQ(copy, sample);
	ASSERT_NE(copy.data(), sample.data());
}

TEST(vector, move_initialisation)
{
	dsa::Vector from(sample);
	dsa::Vector to(std::move(from));

	ASSERT_EQ(from.capacity(), 0ULL);
	ASSERT_NE(to.data(), from.data());
	ASSERT_EQ(to, sample);
}

TEST(vector, comparison_operator_differing_size)
{
	dsa::Vector<int> vector_1(1);
	dsa::Vector<int> vector_2(2);

	ASSERT_NE(vector_1, vector_2);
}

TEST(vector, comparison_operator_differing_element)
{
	dsa::Vector vector_1{0, 0, 0};
	dsa::Vector vector_2{0, 0, 1};
	dsa::Vector vector_3{1, 0, 1};

	ASSERT_NE(vector_1, vector_2);
	ASSERT_NE(vector_2, vector_3);
}

TEST(vector, comparison_operator_equal)
{
	dsa::Vector vector_1{0, -1, 5};
	dsa::Vector vector_2{0, -1, 5};

	ASSERT_EQ(vector_1, vector_2);
}

TEST(vector, comparison_operator_equal_differing_capacity)
{
	dsa::Vector vector_1{0, -1, 5};
	vector_1.reserve(4ULL);

	dsa::Vector vector_2{0, -1, 5};

	ASSERT_EQ(vector_1, vector_2);
}

TEST(vector, copy_assignment)
{
	dsa::Vector<int> copy;
	copy = sample;

	ASSERT_EQ(copy, sample);
	ASSERT_NE(copy.data(), sample.data());
}

TEST(vector, move_assignment)
{
	dsa::Vector      from(sample);
	dsa::Vector<int> to;
	to = std::move(from);

	ASSERT_EQ(from.capacity(), 0ULL);
	ASSERT_NE(to.data(), from.data());
	ASSERT_EQ(to, sample);
}

TEST(vector, access_operator)
{
	dsa::Vector<int> vector(3ULL);

	*(vector.data() + 0ULL) = 1;
	*(vector.data() + 1ULL) = 2;
	*(vector.data() + 2ULL) = 3;

	ASSERT_EQ(vector[0], 1);
	ASSERT_EQ(vector[1], 2);
	ASSERT_EQ(vector[2], 3);
	ASSERT_EQ(vector.front(), 1);
	ASSERT_EQ(vector.back(), 3);
}

TEST(vector, append)
{
	constexpr int value = -4;
	dsa::Vector   vector(sample);
	dsa::Vector   expectation{sample[0], sample[1], sample[2], value};

	vector.append(value);

	ASSERT_EQ(vector, expectation);
}

TEST(vector, insert_front)
{
	constexpr int value = -4;
	dsa::Vector   vector(sample);
	dsa::Vector   expectation{value, sample[0], sample[1], sample[2]};

	vector.insert(0, value);

	ASSERT_EQ(vector, expectation);
}

TEST(vector, insert_middle)
{
	constexpr int value = -4;
	dsa::Vector   vector(sample);
	dsa::Vector   expectation{sample[0], sample[1], value, sample[2]};

	vector.insert(2, value);

	ASSERT_EQ(vector, expectation);
}

TEST(vector, insert_end)
{
	constexpr int value = -4;
	dsa::Vector   vector(sample);
	dsa::Vector   expectation{sample[0], sample[1], sample[2], value};

	vector.insert(vector.size(), value);

	ASSERT_EQ(vector, expectation);
}

TEST(vector, erase_front)
{
	dsa::Vector vector(sample);
	dsa::Vector expectation{sample[1], sample[2]};

	vector.erase(0);

	ASSERT_EQ(vector, expectation);
}

TEST(vector, erase_middle)
{
	dsa::Vector vector(sample);
	dsa::Vector expectation{sample[0], sample[2]};

	vector.erase(1);

	ASSERT_EQ(vector, expectation);
}

TEST(vector, erase_back)
{
	dsa::Vector vector(sample);
	dsa::Vector expectation{sample[0], sample[1]};

	vector.erase(2);

	ASSERT_EQ(vector, expectation);
}

TEST(vector, shirink_to_fit)
{
	constexpr int value = 0;
	dsa::Vector   vector(sample);

	// Force growth
	vector.append(value);

	ASSERT_NE(vector.size(), vector.capacity());

	vector.shrink_to_fit();

	ASSERT_EQ(vector.size(), vector.capacity());
}

TEST(vector, clear)
{
	dsa::Vector vector(sample);

	vector.clear();
	ASSERT_TRUE(vector.empty());
	ASSERT_EQ(vector.capacity(), 0ULL);
}

TEST(vector, resize_to_smaller_size)
{
	dsa::Vector vector(sample);
	dsa::Vector expectation{sample[0], sample[1]};

	vector.resize(expectation.size());
	ASSERT_EQ(vector, expectation);
}

TEST(vector, resize_to_larger_size)
{
	constexpr int default_value = int();
	dsa::Vector   vector(sample);
	dsa::Vector   expectation{
            sample[0],
            sample[1],
            sample[2],
            default_value,
            default_value};

	vector.resize(expectation.size());
	ASSERT_EQ(vector, expectation);
}

TEST(vector, iterate_empty_array)
{
	dsa::Vector<int> empty;
	for ([[maybe_unused]] int value : empty)
	{
		FAIL() << "Expected the body of this loop to not be executed";
	}

	SUCCEED();
}

TEST(vector, iterate_validate_values)
{
	std::size_t index = 0;
	for (int value : sample)
	{
		ASSERT_EQ(value, sample[index++]);
	}
	ASSERT_EQ(index, 3ULL);
}
