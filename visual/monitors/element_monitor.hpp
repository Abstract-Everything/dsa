#ifndef VISUAL_ELEMENT_MONITOR_HPP
#define VISUAL_ELEMENT_MONITOR_HPP

#include "assignment_event.hpp"
#include "event.hpp"

namespace visual
{
class Element_Monitor
{
 public:
	Element_Monitor &operator=(const Element_Monitor &element)
	{
		auto address = reinterpret_cast<std::uint64_t>(this);

		visual::Event::Dispatch(
		    std::make_unique<Assignment_Event>(address));

		if (this == &element)
		{
			return *this;
		}

		return *this;
	}
};

} // namespace visual
#endif
