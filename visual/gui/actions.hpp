#ifndef VISUAL_ACTIONS_HPP
#define VISUAL_ACTIONS_HPP

#include "element_monitor.hpp"
#include "memory_monitor.hpp"

#include <dsa/dynamic_array.hpp>

namespace visual
{

class Actions_UI
{
 public:
	void draw();

 private:
	int m_read  = 0;

	int m_resize = 0;

	int m_value  = 0;
	int m_write  = 0;

	dsa::Dynamic_Array<Element_Monitor<int>, Memory_Monitor<Element_Monitor<int>>>
	    m_dynamic_array{ Memory_Monitor<Element_Monitor<int>>{} };

	void properties();
	void read();
	void resize();
	void write();

	bool is_in_range(std::size_t index);

	void section(const char *label, void (Actions_UI::*interface)());
	void index_input(const char *label, int *value);
};

} // namespace visual

#endif
