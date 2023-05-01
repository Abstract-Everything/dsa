#ifndef TEST_DSA_STATIC_ID_HPP
#define TEST_DSA_STATIC_ID_HPP

#include <cstddef>

namespace test
{

class Id
{
 public:
	explicit constexpr Id() : m_id(0ULL)
	{
	}

	explicit constexpr Id(std::size_t id) : m_id(id)
	{
	}

	[[nodiscard]] constexpr std::size_t value() const
	{
		return m_id;
	}

	[[nodiscard]] constexpr friend bool operator==(Id const &rhs, Id const &lhs)
	{
		return lhs.m_id == rhs.m_id;
	}

	[[nodiscard]] constexpr friend bool operator!=(Id const &rhs, Id const &lhs)
	{
		return !(lhs == rhs);
	}

 private:
	std::size_t m_id;
};

} // namespace test

#endif
