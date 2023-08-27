#ifndef VISUAL_MEMORY_MONITOR_EVENT_REGISTRATOR_SCOPE_HPP
#define VISUAL_MEMORY_MONITOR_EVENT_REGISTRATOR_SCOPE_HPP

namespace visual
{

class Enable_Event_Registration_Scope
{
 public:
	static void block_events();

	Enable_Event_Registration_Scope();
	~Enable_Event_Registration_Scope();

	Enable_Event_Registration_Scope(Enable_Event_Registration_Scope const &) = delete;

	Enable_Event_Registration_Scope &operator=(Enable_Event_Registration_Scope const &) = delete;

	Enable_Event_Registration_Scope(Enable_Event_Registration_Scope &&) = delete;

	Enable_Event_Registration_Scope &operator=(Enable_Event_Registration_Scope &&) = delete;
};

} // namespace visual

#endif
