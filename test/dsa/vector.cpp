#include <dsa/vector.hpp>

#include <gtest/gtest.h>

static_assert(std::is_same_v<dsa::Vector<int>::Value, int>);

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
	ASSERT_NE(copy.data().get(), sample.data().get());
}

TEST(vector, move_initialisation)
{
	dsa::Vector from(sample);
	dsa::Vector to(std::move(from));

	ASSERT_EQ(from.capacity(), 0ULL);
	ASSERT_NE(to.data().get(), from.data().get());
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
	ASSERT_NE(copy.data().get(), sample.data().get());
}

TEST(vector, move_assignment)
{
	dsa::Vector      from(sample);
	dsa::Vector<int> to;
	to = std::move(from);

	ASSERT_EQ(from.capacity(), 0ULL);
	ASSERT_NE(to.data().get(), from.data().get());
	ASSERT_EQ(to, sample);
}

TEST(vector, access_operator)
{
	dsa::Vector<int> vector(3ULL);

	*(vector.data().get() + 0ULL) = 1;
	*(vector.data().get() + 1ULL) = 2;
	*(vector.data().get() + 2ULL) = 3;

	ASSERT_EQ(vector[0], 1);
	ASSERT_EQ(vector[1], 2);
	ASSERT_EQ(vector[2], 3);
	ASSERT_EQ(vector.front(), 1);
	ASSERT_EQ(vector.back(), 3);
}

