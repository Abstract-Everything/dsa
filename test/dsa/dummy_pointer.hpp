#ifndef TEST_DSA_STATIC_DUMMY_POINTER_HPP
#define TEST_DSA_STATIC_DUMMY_POINTER_HPP

#include "id.hpp"

namespace test
{

class Dummy_Pointer
{
 public:
	explicit constexpr Dummy_Pointer(Id id) : m_id(id) {
	}

	constexpr ~Dummy_Pointer() = default;

	Dummy_Pointer(Dummy_Pointer const &)            = delete;
	Dummy_Pointer(Dummy_Pointer &&)                 = default;
	Dummy_Pointer &operator=(Dummy_Pointer const &) = delete;
	Dummy_Pointer &operator=(Dummy_Pointer &&)      = default;

	[[nodiscard]] constexpr friend bool operator==(Dummy_Pointer const &lhs, Dummy_Pointer const &rhs) {
		return lhs.m_id == rhs.m_id;
	}

	[[nodiscard]] constexpr friend bool operator!=(Dummy_Pointer const &lhs, Dummy_Pointer const &rhs) {
		return !(lhs == rhs);
	}

	[[nodiscard]] constexpr Id id() const {
		return m_id;
	}

 private:
	Id m_id;
};

} // namespace test

#endif
