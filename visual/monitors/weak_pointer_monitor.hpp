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

	Weak_Pointer_Monitor &operator=(std::nullptr_t) noexcept
	{
		m_pointer = nullptr;
		dispatch_copy();
		return *this;
	}

	bool operator==(std::nullptr_t) const
	{
		return m_pointer == nullptr;
	}

	bool operator!=(std::nullptr_t) const
	{
		return !this->operator==(nullptr);
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

	void dispatch_copy()
	{
		Dispatch(Copy_Assignment_Event(
		    to_raw_address(this),
		    Memory_Value{ sizeof(*this),
				  true,
				  std::to_string(to_raw_address(m_pointer)) }));
	}

	void dispatch_move(const Weak_Pointer_Monitor &pointer)
	{
		Dispatch(Move_Assignment_Event(
		    to_raw_address(this),
		    to_raw_address(&pointer),
		    Memory_Value{ sizeof(*this),
				  true,
				  std::to_string(to_raw_address(m_pointer)) }));
	}
};

} // namespace visual

#endif
