#ifndef VISUAL_MONITOR_HPP
#define VISUAL_MONITOR_HPP

#include "address.hpp"
#include "allocated_array_event.hpp"
#include "deallocated_array_event.hpp"
#include "event.hpp"

#include <dsa/memory_monitor.hpp>

#include <sstream>

namespace visual
{

class Visual_Monitor
{
 public:
	template<typename T>
	static auto before_deallocate(dsa::Allocation_Event<T> /* event */)
	    -> bool
	{
		return true;
	}

	template<typename T>
	static void process_allocation_event(dsa::Allocation_Event<T> event)
	{
		switch (event.type())
		{
		case dsa::Allocation_Event_Type::Allocate:
			visual::Dispatch(Allocated_Array_Event{
			    to_raw_address(event.address()),
			    sizeof(T),
			    event.count()});
			break;

		case dsa::Allocation_Event_Type::Deallocate:
			visual::Dispatch(Deallocated_Array_Event(
			    to_raw_address(event.address())));
			break;
		}
	}

	template<typename T>
	static void process_object_event(dsa::Object_Event<T> event)
	{
		std::stringstream stream;
		stream << *event.destination();

		switch (event.type())
		{
		case dsa::Object_Event_Type::Before_Construct:
			break;

		case dsa::Object_Event_Type::Construct:
		case dsa::Object_Event_Type::Copy_Construct:
		case dsa::Object_Event_Type::Copy_Assign:
		case dsa::Object_Event_Type::Underlying_Copy_Assign:
			visual::Dispatch(Copy_Assignment_Event{
			    to_raw_address(event.destination()),
			    Memory_Value{sizeof(T), true, stream.str()}
                        });
			break;

		case dsa::Object_Event_Type::Move_Construct:
		case dsa::Object_Event_Type::Move_Assign:
		case dsa::Object_Event_Type::Underlying_Move_Assign:
			visual::Dispatch(Move_Assignment_Event{
			    to_raw_address(event.destination()),
			    to_raw_address(event.source()),
			    Memory_Value{sizeof(T), true, stream.str()}
                        });
			break;

		case dsa::Object_Event_Type::Destroy:
			visual::Dispatch(Copy_Assignment_Event{
			    to_raw_address(event.destination()),
			    Memory_Value{sizeof(T)}
                        });
			break;
		}
	}
};

} // namespace visual

#endif
