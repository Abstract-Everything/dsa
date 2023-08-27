#ifndef TEST_DSA_STATIC_INCOMPARABLE_VALUE_HPP
#define TEST_DSA_STATIC_INCOMPARABLE_VALUE_HPP

#include <compare>

class Incomparable_Value
{
 public:
	explicit Incomparable_Value(int value) : m_value(value) {
	}

	[[nodiscard]] std::strong_ordering compare(Incomparable_Value const &value) const {
		return m_value <=> value.m_value;
	}

	std::strong_ordering operator<=>(Incomparable_Value const &value) = delete;

 private:
	int m_value;
};

#endif
