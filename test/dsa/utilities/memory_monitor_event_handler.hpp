#ifndef TEST_DSA_STATIC_MEMORY_MONITOR_EVENT_HANDLER_HPP
#define TEST_DSA_STATIC_MEMORY_MONITOR_EVENT_HANDLER_HPP

#include "empty_value.hpp"
#include "no_default_constructor_value.hpp"

#include <dsa/memory_monitor.hpp>

#include <utilities/memory_monitor_handler_scope.hpp>

#include <optional>
#include <variant>
#include <vector>

namespace test
{

using Event_Type = std::variant<
    std::monostate,
    dsa::Allocation_Event<Empty_Value>,
    dsa::Object_Event<Empty_Value>,
    dsa::Object_Event<Empty_Value *>,
    dsa::Allocation_Event<No_Default_Constructor_Value>,
    dsa::Object_Event<No_Default_Constructor_Value>,
    dsa::Object_Event<No_Default_Constructor_Value *>>;

inline auto operator<<(std::ostream &stream, Event_Type const &event) -> std::ostream & {
	std::visit(
	    dsa::Overloaded_Lambda{
		[&](std::monostate const &) { stream << "<not-found>"; },
		[&](auto const &typed_event) { stream << typed_event; }},
	    event);
	return stream;
}

/// Maintains a list events produced by the appropriate callback. The entries
/// can be analysed to determine if the Memory_Monitor is calling the
/// appropriate callbacks in the appropriate order
class Event_Handler
{
 public:
	static auto instance() -> std::unique_ptr<Event_Handler> & {
		static std::unique_ptr<Event_Handler> instance = nullptr;
		return instance;
	}

	template<typename T>
	static auto before_deallocate(dsa::Allocation_Event<T> /* event */) -> bool {
		return instance()->m_allow_deallocate;
	}

	template<typename T>
	static void process_allocation_event(dsa::Allocation_Event<T> event) {
		instance()->m_events.push_back(event);
	}

	template<typename T>
	static void process_object_event(dsa::Object_Event<T> event) {
		instance()->m_events.push_back(event);
	}

	void cleanup() {
	}

	void block_deallocate() {
		m_allow_deallocate = false;
	}

	void unblock_deallocate() {
		m_allow_deallocate = true;
	}

	[[nodiscard]] static auto events() -> std::vector<Event_Type> const & {
		return instance()->m_events;
	}

	template<dsa::Memory_Monitor_Event T>
	[[nodiscard]] auto last_event(std::optional<typename T::Event_Type> const &type = std::nullopt) const
	    -> Event_Type {
		for (auto it = m_events.rbegin(); it != m_events.rend(); ++it)
		{
			bool const found = std::visit(
			    dsa::Overloaded_Lambda{
				[](auto &&) -> bool { return false; },
				[&](T const &instance) -> bool {
					return !type.has_value() || instance.type() == type.value();
				}},
			    *it);

			if (found)
			{
				return *it;
			}
		}
		return std::monostate{};
	}

 private:
	std::vector<Event_Type> m_events;
	bool                    m_allow_deallocate = true;
};

} // namespace test

#endif
