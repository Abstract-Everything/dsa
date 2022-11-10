#ifndef TEST_DSA_STATIC_MEMORY_MONITOR_SCOPE_HPP
#define TEST_DSA_STATIC_MEMORY_MONITOR_SCOPE_HPP

#include <dsa/memory_monitor.hpp>

#include <iostream>

namespace test
{

/// The Memory_Monitor class uses a global variable to handle the callbacks.
/// This class can be used to manage the lifetime of the global instance. This
/// is useful for tests since we want each test to have a separate instance.
template<typename Type, dsa::Memory_Monitor_Event_Handler<Type> Event_Handler>
class Memory_Monitor_Scope
{
 public:
	Memory_Monitor_Scope()
	{
		if (Allocator::handler() != nullptr)
		{
			std::cerr << "The handler's lifetime should only be "
				     "controlled "
				     "by this object";
			std::terminate();
		}

		Allocator::handler() = std::make_unique<Event_Handler>();
	}

	~Memory_Monitor_Scope()
	{
		if (Allocator::handler() == nullptr)
		{
			std::cerr << "The handler's lifetime should only be "
				     "controlled "
				     "by this object";
			std::terminate();
		}

		Allocator::handler() = nullptr;
	}

	using Allocator = dsa::Memory_Monitor<Type, Event_Handler>;
};

} // namespace test

#endif
