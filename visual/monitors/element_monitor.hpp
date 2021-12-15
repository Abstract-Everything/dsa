#ifndef VISUAL_ELEMENT_MONITOR_HPP
#define VISUAL_ELEMENT_MONITOR_HPP

#include "address.hpp"
#include "copy_assignment_event.hpp"
#include "event.hpp"
#include "move_assignment_event.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>

namespace visual
{

template<typename Value_t>
class Element_Monitor
{
	using Value = Value_t;

 public:
	Element_Monitor() : Element_Monitor{ 0 }
	{
	}

	explicit Element_Monitor(Value value)
	    : m_initialised(true)
	    , m_value(std::move(value))
	{
	}

	Element_Monitor(const Element_Monitor &element)
	    : m_initialised(element.m_initialised)
	    , m_value(element.m_value)
	{
	}

	Element_Monitor(Element_Monitor &&element) noexcept
	    : m_initialised(element.m_initialised)
	    , m_value(std::move(element.m_value))
	{
	}

	void uninitialize()
	{
		m_initialised = false;
	}

	friend void swap(Element_Monitor &lhs, Element_Monitor &rhs)
	{
		std::swap(lhs.m_initialised, rhs.m_initialised);
		std::swap(lhs.m_value, rhs.m_value);
	}

	Element_Monitor &operator=(const Element_Monitor &element)
	{
		Element_Monitor temp{ element };
		swap(*this, temp);

		visual::Dispatch(Copy_Assignment_Event{
		    to_raw_address(this),
		    Memory_Value{ m_initialised, to_string() } });

		return *this;
	}

	Element_Monitor &operator=(Element_Monitor &&element) noexcept
	{
		// We leave element in a good state so that it could dispatch
		// correct events if copied after move
		Element_Monitor tmp{};
		swap(*this, tmp);
		swap(*this, element);

		visual::Dispatch(Move_Assignment_Event{
		    to_raw_address(this),
		    to_raw_address(&element),
		    Memory_Value{ m_initialised, to_string() } });

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

 private:
	bool  m_initialised = false;
	Value m_value{};
};

} // namespace visual
#endif
