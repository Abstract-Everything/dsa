#ifndef DSA_WEAK_POINTER_HPP
#define DSA_WEAK_POINTER_HPP

#include <cstddef>
#include <utility>

namespace dsa
{

template<typename Value>
class Weak_Pointer
{
 public:
	Weak_Pointer() = default;

	Weak_Pointer(Value *pointer) : m_pointer(pointer)
	{
	}

	~Weak_Pointer() = default;

	Weak_Pointer(const Weak_Pointer &pointer) : m_pointer(pointer.m_pointer)
	{
	}

	Weak_Pointer(Weak_Pointer &&pointer) noexcept : Weak_Pointer()
	{
		swap(*this, pointer);
	}

	friend void swap(Weak_Pointer &lhs, Weak_Pointer &rhs)
	{
		std::swap(lhs.m_pointer, rhs.m_pointer);
	}

	Weak_Pointer &operator=(Weak_Pointer pointer) noexcept
	{
		swap(*this, pointer);
		return *this;
	}

	Weak_Pointer &operator=(Value *pointer) noexcept
	{
		m_pointer = pointer;
		return *this;
	}

	Weak_Pointer &operator=(std::nullptr_t) noexcept
	{
		m_pointer = nullptr;
		return *this;
	}

	bool operator==(const Weak_Pointer& pointer) const
	{
		return m_pointer == pointer.m_pointer;
	}

	bool operator!=(const Weak_Pointer& pointer) const
	{
		return !this->operator==(pointer);
	}

	Value &operator*()
	{
		return *m_pointer;
	}

	const Value &operator*() const
	{
		return *m_pointer;
	}

	Value *operator->()
	{
		return m_pointer;
	}

	const Value *operator->() const
	{
		return m_pointer;
	}

	Value *get()
	{
		return m_pointer;
	}

	const Value *get() const
	{
		return m_pointer;
	}

 private:
	Value *m_pointer = nullptr;
};

} // namespace dsa

#endif
