#ifndef VISUAL_WEAK_POINTER_MONITOR_HPP
#define VISUAL_WEAK_POINTER_MONITOR_HPP

#include "copy_assignment_event.hpp"
#include "event.hpp"
#include "move_assignment_event.hpp"

#include <utility>

namespace visual
{

template<typename Value>
class Weak_Pointer_Monitor
{
 public:
	using iterator_category = std::random_access_iterator_tag;
	using difference_type   = std::ptrdiff_t;
	using value_type        = Value;
	using reference         = Value &;
	using pointer           = Weak_Pointer_Monitor;

	Weak_Pointer_Monitor() = default;

	Weak_Pointer_Monitor(Value *pointer) : m_pointer(pointer)
	{
	}

	~Weak_Pointer_Monitor() = default;

	Weak_Pointer_Monitor(const Weak_Pointer_Monitor &pointer)
	    : m_pointer(pointer.m_pointer)
	{
	}

	Weak_Pointer_Monitor(Weak_Pointer_Monitor &&pointer) noexcept
	    : Weak_Pointer_Monitor()
	{
		swap(*this, pointer);
		dispatch_move(pointer);
	}

	friend void swap(Weak_Pointer_Monitor &lhs, Weak_Pointer_Monitor &rhs)
	{
		std::swap(lhs.m_pointer, rhs.m_pointer);
	}

	Weak_Pointer_Monitor &operator=(const Weak_Pointer_Monitor &pointer)
	{
		m_pointer = pointer.m_pointer;
		dispatch_copy();
		return *this;
	}

	Weak_Pointer_Monitor &operator=(Weak_Pointer_Monitor &&pointer) noexcept
	{
		swap(*this, pointer);
		dispatch_move(pointer);
		return *this;
	}

	Weak_Pointer_Monitor &operator=(Value *pointer) noexcept
	{
		m_pointer = pointer;
		dispatch_copy();
		return *this;
	}

	Weak_Pointer_Monitor operator++()
	{
		m_pointer++;
		Weak_Pointer_Monitor pointer(m_pointer);
		return pointer;
	}

	const Weak_Pointer_Monitor operator++(int)
	{
		Weak_Pointer_Monitor pointer(m_pointer);
		m_pointer++;
		return pointer;
	}

	// ToDo: Use concepts to filter T
	template<typename T>
	friend Weak_Pointer_Monitor operator+(
	    Weak_Pointer_Monitor const &pointer,
	    T                           offset)
	{
		return pointer.m_pointer + offset;
	}

	// ToDo: Use concepts to filter T
	friend difference_type operator-(
	    Weak_Pointer_Monitor const &pointer,
	    Weak_Pointer_Monitor const &offset)
	{
		return pointer.m_pointer - offset.m_pointer;
	}

	template<typename T>
	friend difference_type operator-(Weak_Pointer_Monitor const &pointer, T offset)
	{
		return pointer.m_pointer - offset;
	}

	bool operator==(const Weak_Pointer_Monitor &pointer) const
	{
		return this->operator==(pointer.m_pointer);
	}

	bool operator==(Value *pointer) const
	{
		return m_pointer == pointer;
	}

	bool operator!=(const Weak_Pointer_Monitor &pointer) const
	{
		return !this->operator==(pointer.m_pointer);
	}

	bool operator!=(Value *pointer) const
	{
		return !this->operator==(pointer);
	}

	template<typename T>
	Value &operator[](T index) const
	{
		return m_pointer[index];
	}

	Value &operator*() const
	{
		return *m_pointer;
	}

	Value *operator->() const
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

	void dispatch_copy()
	{
		Dispatch(Copy_Assignment_Event(
		    to_raw_address(this),
		    Memory_Value{sizeof(*this), true, to_raw_address(m_pointer)}));
	}

	void dispatch_move(const Weak_Pointer_Monitor &pointer)
	{
		Dispatch(Move_Assignment_Event(
		    to_raw_address(this),
		    to_raw_address(&pointer),
		    Memory_Value{sizeof(*this), true, to_raw_address(m_pointer)}));
	}
};

} // namespace visual

#endif
