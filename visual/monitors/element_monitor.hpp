#ifndef VISUAL_ELEMENT_MONITOR_HPP
#define VISUAL_ELEMENT_MONITOR_HPP

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
	Element_Monitor() = default;

	explicit Element_Monitor(Value value)
	    : m_initialised(true)
	    , m_value(std::move(value))
	{
	}

	friend void swap(Element_Monitor &lhs, Element_Monitor &rhs)
	{
		std::swap(lhs.m_initialised, rhs.m_initialised);
		std::swap(lhs.m_value, rhs.m_value);
	}

	Element_Monitor &operator=(Element_Monitor &&element)
	{
		swap(*this, element);

		auto address = reinterpret_cast<std::uint64_t>(this);

		visual::Event::Dispatch(std::make_unique<Move_Assignment_Event>(
		    m_initialised,
		    address,
		    to_string()));

		return *this;
	}

	std::ostream &operator<<(std::ostream &stream) const
	{
		stream << to_string();
		return stream;
	}

 private:
	bool  m_initialised = false;
	Value m_value{};

	[[nodiscard]] std::string to_string() const
	{
		return std::to_string(m_value);
	}
};

} // namespace visual
#endif
