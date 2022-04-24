#ifndef VISUAL_ELEMENT_MONITOR_HPP
#define VISUAL_ELEMENT_MONITOR_HPP

#include "address.hpp"
#include "copy_assignment_event.hpp"
#include "event.hpp"
#include "move_assignment_event.hpp"
#include "swap_event.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>

namespace visual
{

template<typename Value>
class Element_Monitor
{
 public:
	Element_Monitor() : Element_Monitor{0}
	{
	}

	explicit Element_Monitor(Value value)
	    : m_initialised(true)
	    , m_value(std::move(value))
	{
	}

	~Element_Monitor() = default;

	Element_Monitor(const Element_Monitor &element)
	    : m_initialised(element.m_initialised)
	    , m_value(element.m_value)
	{
		dispatch_copy();
	}

	Element_Monitor(Element_Monitor &&element) noexcept
	    : m_initialised(element.m_initialised)
	    , m_value(std::move(element.m_value))
	{
		dispatch_move(element);
	}

	friend void swap(Element_Monitor &lhs, Element_Monitor &rhs)
	{
		using std::swap;
		swap(lhs.m_initialised, rhs.m_initialised);
		swap(lhs.m_value, rhs.m_value);

		dispatch_swap(lhs, rhs);
	}

	Element_Monitor &operator=(const Element_Monitor &element)
	{
		Element_Monitor temp{element};
		swap(*this, temp);

		dispatch_copy();

		return *this;
	}

	Element_Monitor &operator=(Element_Monitor &&element) noexcept
	{
		// We leave element in a good state so that it could dispatch
		// correct events if copied after move
		Element_Monitor tmp{};
		swap(*this, tmp);
		swap(*this, element);

		dispatch_move(element);

		return *this;
	}

	std::ostream &operator<<(std::ostream &stream) const
	{
		stream << to_string();
		return stream;
	}

	[[nodiscard]] std::string to_string() const
	{
		if (m_initialised)
		{
			return std::to_string(m_value);
		}

		return "<Uninitialized memory>";
	}

	bool operator<(const Element_Monitor<Value> element) const
	{
		return m_value < element.m_value;
	}

	bool operator>(const Element_Monitor<Value> element) const
	{
		return m_value > element.m_value;
	}

	bool operator==(const Element_Monitor<Value> element) const
	{
		return m_value == element.m_value;
	}

	bool operator!=(const Element_Monitor<Value> element) const
	{
		return m_value != element.m_value;
	}

	void uninitialise()
	{
		m_initialised = false;
	}

 private:
	bool  m_initialised = false;
	Value m_value{};

	static void dispatch_swap(const Element_Monitor &lhs, const Element_Monitor &rhs)
	{
		visual::Dispatch(Swap_Event(
		    to_raw_address(&lhs),
		    Memory_Value(sizeof(lhs), lhs.m_initialised, lhs.to_string()),
		    to_raw_address(&rhs),
		    Memory_Value(sizeof(rhs), rhs.m_initialised, rhs.to_string())));
	}

	void dispatch_copy()
	{
		visual::Dispatch(Copy_Assignment_Event{
		    to_raw_address(this),
		    Memory_Value{sizeof(*this), m_initialised, to_string()}
                });
	}

	void dispatch_move(const Element_Monitor &element)
	{
		visual::Dispatch(Move_Assignment_Event{
		    to_raw_address(this),
		    to_raw_address(&element),
		    Memory_Value{sizeof(*this), m_initialised, to_string()}
                });
	}
};

} // namespace visual
#endif
