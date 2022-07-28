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
	// MSVC version 19.29 gives an error if this is inlined.
	template<typename T>
	static constexpr bool Enable_Conversion_To_Const_V =
	    std::is_const_v<Value> && !std::is_const_v<T>;

	template<typename T>
	using Enable_Conversion_To_Const =
	    typename std::enable_if_t<Enable_Conversion_To_Const_V<T>, bool>;

 public:
	using iterator_category = std::random_access_iterator_tag;
	using difference_type   = std::ptrdiff_t;
	using value_type        = Value;
	using reference         = Value &;
	using pointer           = Weak_Pointer_Monitor;

	Weak_Pointer_Monitor() = default;

	Weak_Pointer_Monitor(Value *weak_pointer) : m_pointer(weak_pointer)
	{
	}

	template<typename Value_t, Enable_Conversion_To_Const<Value_t> = true>
	Weak_Pointer_Monitor(Weak_Pointer_Monitor<Value_t> const &weak_pointer)
	    : m_pointer(weak_pointer.get())
	{
	}

	~Weak_Pointer_Monitor() = default;

	Weak_Pointer_Monitor(const Weak_Pointer_Monitor &weak_pointer)
	    : m_pointer(weak_pointer.m_pointer)
	{
	}

	Weak_Pointer_Monitor(Weak_Pointer_Monitor &&weak_pointer) noexcept
	    : Weak_Pointer_Monitor()
	{
		swap(*this, weak_pointer);
		dispatch_move(weak_pointer);
	}

	friend void swap(Weak_Pointer_Monitor &lhs, Weak_Pointer_Monitor &rhs)
	{
		using std::swap;
		swap(lhs.m_pointer, rhs.m_pointer);
	}

	Weak_Pointer_Monitor &operator=(const Weak_Pointer_Monitor &weak_pointer)
	{
		if (this != &weak_pointer)
		{
			m_pointer = weak_pointer.m_pointer;
		}
		dispatch_copy();
		return *this;
	}

	Weak_Pointer_Monitor &operator=(Weak_Pointer_Monitor &&weak_pointer) noexcept
	{
		swap(*this, weak_pointer);
		dispatch_move(weak_pointer);
		return *this;
	}

	Weak_Pointer_Monitor &operator=(Value *weak_pointer) noexcept
	{
		if (m_pointer != weak_pointer)
		{
			m_pointer = weak_pointer;
		}
		dispatch_copy();
		return *this;
	}

	template<typename Value_t, Enable_Conversion_To_Const<Value_t> = true>
	Weak_Pointer_Monitor &operator=(
	    Weak_Pointer_Monitor<Value_t> const &weak_pointer) noexcept
	{
		m_pointer = weak_pointer.get();
		dispatch_copy();
		return *this;
	}

	Weak_Pointer_Monitor operator++()
	{
		m_pointer++;
		Weak_Pointer_Monitor weak_pointer(m_pointer);
		return weak_pointer;
	}

	const Weak_Pointer_Monitor operator++(int)
	{
		Weak_Pointer_Monitor weak_pointer(m_pointer);
		m_pointer++;
		return weak_pointer;
	}

	Weak_Pointer_Monitor operator--()
	{
		m_pointer--;
		Weak_Pointer_Monitor weak_pointer(m_pointer);
		return weak_pointer;
	}

	const Weak_Pointer_Monitor operator--(int)
	{
		Weak_Pointer_Monitor weak_pointer(m_pointer);
		m_pointer--;
		return weak_pointer;
	}

	// ToDo: Use concepts to filter T
	template<typename T>
	friend Weak_Pointer_Monitor operator+(
	    Weak_Pointer_Monitor const &weak_pointer,
	    T                           offset)
	{
		return weak_pointer.m_pointer + offset;
	}

	// ToDo: Use concepts to filter T
	friend difference_type operator-(
	    Weak_Pointer_Monitor const &weak_pointer,
	    Weak_Pointer_Monitor const &offset)
	{
		return weak_pointer.m_pointer - offset.m_pointer;
	}

	template<typename T>
	friend Weak_Pointer_Monitor operator-(Weak_Pointer_Monitor const &weak_pointer, T offset)
	{
		return weak_pointer.m_pointer - offset;
	}

	bool operator==(const Weak_Pointer_Monitor &weak_pointer) const
	{
		return this->operator==(weak_pointer.m_pointer);
	}

	bool operator==(Value *weak_pointer) const
	{
		return m_pointer == weak_pointer;
	}

	bool operator!=(const Weak_Pointer_Monitor &weak_pointer) const
	{
		return !this->operator==(weak_pointer.m_pointer);
	}

	bool operator!=(Value *weak_pointer) const
	{
		return !this->operator==(weak_pointer);
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

	void dispatch_move(const Weak_Pointer_Monitor &weak_pointer)
	{
		Dispatch(Move_Assignment_Event(
		    to_raw_address(this),
		    to_raw_address(&weak_pointer),
		    Memory_Value{sizeof(*this), true, to_raw_address(m_pointer)}));
	}
};

} // namespace visual

#endif
