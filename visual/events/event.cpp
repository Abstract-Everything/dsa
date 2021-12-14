
#include "event.hpp"

#include "main_window.hpp"

namespace visual
{

void Dispatch(Event &&event)
{
	Main_Window::instance().add_event(std::move(event));
}

} // namespace visual
