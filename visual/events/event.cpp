#include "event.hpp"

#include "main_window.hpp"

namespace visual
{

void Dispatch(Event &&event)
{
	Main_Window::instance().add_event(std::move(event));
}

std::string to_string(const Event &event)
{
	return std::visit(
	    [](auto &&event_typed) { return event_typed.to_string(); },
	    event);
}

} // namespace visual
