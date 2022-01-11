#include <dsa/list.hpp>

#include <gtest/gtest.h>

static_assert(std::is_same_v<dsa::List<int>::Value, int>);

namespace
{

const dsa::List<int> sample{0, 1, 2};

}

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

TEST(list, copy_initialisation)
{
	dsa::List list(sample);

	ASSERT_EQ(list, sample);
	ASSERT_NE(&list[0], &sample[0]);
}

TEST(list, move_initialisation)
{
	dsa::List from(sample);
	dsa::List to(std::move(from));

	ASSERT_TRUE(from.empty());
	ASSERT_EQ(to, sample);
}

TEST(list, copy_assignment)
{
	dsa::List<int> list;
	list = sample;

	ASSERT_EQ(list, sample);
	ASSERT_NE(&list[0], &sample[0]);
}

TEST(list, move_assignment)
{
	dsa::List      from(sample);
	dsa::List<int> to;
	to = std::move(from);

	ASSERT_TRUE(from.empty());
	ASSERT_EQ(to, sample);
}

TEST(list, clear)
{
	dsa::List list(sample);

	ASSERT_FALSE(list.empty());

	list.clear();

	ASSERT_TRUE(list.empty());
	ASSERT_EQ(list.size(), 0ULL);
}

TEST(list, prepend)
{
	constexpr int value = -1;
	dsa::List     list(sample);
	dsa::List     expected{value, sample[0], sample[1], sample[2]};

	list.prepend(value);

	ASSERT_EQ(list, expected);
}

TEST(list, insert_front)
{
	constexpr int value = -1;
	dsa::List     list(sample);
	dsa::List     expected{value, sample[0], sample[1], sample[2]};

	list.insert(0, value);

	ASSERT_EQ(list, expected);
}

TEST(list, insert_middle)
{
	constexpr int value = -1;
	dsa::List     list(sample);
	dsa::List     expected{sample[0], sample[1], value, sample[2]};

	list.insert(2, value);

	ASSERT_EQ(list, expected);
}

TEST(list, insert_back)
{
	constexpr int value = -1;
	dsa::List     list(sample);
	dsa::List     expected{sample[0], sample[1], sample[2], value};

	list.insert(list.size(), value);

	ASSERT_EQ(list, expected);
}

TEST(list, detatch_front)
{
	dsa::List list(sample);
	dsa::List expected{sample[1], sample[2]};

	list.detatch_front();

	ASSERT_EQ(list, expected);
}
