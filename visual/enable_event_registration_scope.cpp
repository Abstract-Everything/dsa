#include "enable_event_registration_scope.hpp"

#include "main_window.hpp"

namespace visual
{

void Enable_Event_Registration_Scope::block_events()
{
	Main_Window::instance().registering(false);
}

Enable_Event_Registration_Scope::Enable_Event_Registration_Scope()
{
	assert(
	    !Main_Window::instance().is_registering()
	    && "Multiple active scopes are not supported");
	Main_Window::instance().registering(true);
}

Enable_Event_Registration_Scope::~Enable_Event_Registration_Scope()
{
	block_events();
}

} // namespace visual
