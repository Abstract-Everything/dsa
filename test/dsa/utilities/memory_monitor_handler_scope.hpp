#ifndef TEST_DSA_STATIC_MEMORY_MONITOR_SCOPE_HPP
#define TEST_DSA_STATIC_MEMORY_MONITOR_SCOPE_HPP

#include <iostream>
#include <memory>

namespace test
{

template<typename Handler>
concept Memory_Monitor_Scope_Event_Handler = requires() {
	{ Handler::instance() } -> std::same_as<std::unique_ptr<Handler> &>;
	{ Handler::instance()->cleanup() };
};

/// The Memory_Monitor class uses a global variable to handle the callbacks.
/// This class can be used to manage the lifetime of the global instance. This
/// is useful for tests since we want each test to have a separate instance.
template<Memory_Monitor_Scope_Event_Handler Handler>
class Memory_Monitor_Handler_Scope
{
 public:
	Memory_Monitor_Handler_Scope() {
		if (Handler::instance() != nullptr)
		{
			std::cerr << "The handler's lifetime should only be "
				     "controlled by this object";
			std::terminate();
		}

		Handler::instance() = std::make_unique<Handler>();
	}

	~Memory_Monitor_Handler_Scope() {
		if (Handler::instance() == nullptr)
		{
			std::cerr << "The handler's lifetime should only be "
				     "controlled "
				     "by this object";
			std::terminate();
		}

		Handler::instance()->cleanup();
		Handler::instance() = nullptr;
	}

	Memory_Monitor_Handler_Scope(Memory_Monitor_Handler_Scope const &) = delete;
	auto operator=(Memory_Monitor_Handler_Scope const &)
	    -> Memory_Monitor_Handler_Scope & = delete;

	Memory_Monitor_Handler_Scope(Memory_Monitor_Handler_Scope &&) = delete;
	auto operator=(Memory_Monitor_Handler_Scope &&)
	    -> Memory_Monitor_Handler_Scope & = delete;
};

} // namespace test

#endif
