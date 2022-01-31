#include <dsa/list.hpp>

#include <gtest/gtest.h>

using Int_List = dsa::List<int>;

static_assert(std::is_same_v<Int_List::Value, int>);

static_assert(std::is_same_v<decltype(*std::declval<Int_List>().begin()), int &>);
static_assert(std::is_same_v<decltype(*std::declval<const Int_List>().begin()), const int &>);
static_assert(std::is_same_v<decltype(*std::declval<Int_List>().end()), int &>);
static_assert(std::is_same_v<decltype(*std::declval<const Int_List>().end()), const int &>);

namespace
{

const dsa::List<int> sample{0, 1, 2};
const dsa::List<int> long_sample{0, 1, 2, 3, 4};

} // namespace

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

TEST(list, swap)
{
	dsa::List list_1(sample);
	dsa::List list_2(long_sample);

	swap(list_1, list_2);

	ASSERT_EQ(list_1, long_sample);
	ASSERT_EQ(list_2, sample);
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

TEST(list, erase_front)
{
	dsa::List     list(sample);
	dsa::List     expected{sample[1], sample[2]};

	list.erase(0);

	ASSERT_EQ(list, expected);
}

TEST(list, erase_middle)
{
	dsa::List     list(sample);
	dsa::List     expected{sample[0], sample[2]};

	list.erase(1);

	ASSERT_EQ(list, expected);
}

TEST(list, erase_back)
{
	dsa::List     list(sample);
	dsa::List     expected{sample[0], sample[1]};

	list.erase(2);

	ASSERT_EQ(list, expected);
}

TEST(list, comparison_operator_empty_lists)
{
	dsa::List<int> list_1;
	dsa::List<int> list_2;

	ASSERT_EQ(list_1, list_2);
}

TEST(list, comparison_operator_differing_element)
{
	dsa::List list_1{0};
	dsa::List list_2{1};

	ASSERT_NE(list_1, list_2);
}

TEST(list, comparison_operator_same_single_element)
{
	dsa::List<int> list_1{0};
	dsa::List<int> list_2{0};

	ASSERT_EQ(list_1, list_2);
}

TEST(list, comparison_operator_same_multiple_elements)
{
	dsa::List<int> list_1{0, 1, 2};
	dsa::List<int> list_2{0, 1, 2};

	ASSERT_EQ(list_1, list_2);
}

TEST(list, comparison_operator_single_differing_element)
{
	dsa::List<int> list_1{0, 1, 2};
	dsa::List<int> list_2{0, 1, 3};

	ASSERT_NE(list_1, list_2);
}

TEST(list, comparison_operator_differing_size)
{
	dsa::List<int> list_1{0, 1};
	dsa::List<int> list_2{0, 1, 2};

	ASSERT_NE(list_1, list_2);
}

TEST(list, destroy_elements)
{
	constexpr std::size_t length  = 3;
	std::shared_ptr<int>  counter = std::make_shared<int>(0);
	{
		dsa::List<std::shared_ptr<int>> list;
		for (std::size_t i = 0; i < length; ++i)
		{
			list.prepend(counter);
		}
		ASSERT_EQ(counter.use_count(), length + 1);
	}

	ASSERT_EQ(counter.use_count(), 1);
}

TEST(list, iterate_empty_list)
{
	dsa::List<int> empty;
	for ([[maybe_unused]] int value : empty)
	{
		FAIL() << "Expected the body of this loop to not be executed";
	}

	SUCCEED();
}

TEST(list, iterate_validate_values)
{
	std::size_t index = 0;
	for (int value : sample)
	{
		ASSERT_EQ(value, sample[index++]);
	}
	ASSERT_EQ(index, 3ULL);
}
