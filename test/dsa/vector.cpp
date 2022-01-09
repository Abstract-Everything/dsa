#include <dsa/vector.hpp>

#include <gtest/gtest.h>

static_assert(std::is_same_v<dsa::Vector<int>::Value, int>);

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
