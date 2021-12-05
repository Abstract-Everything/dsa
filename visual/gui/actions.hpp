#ifndef VISUAL_ACTIONS_HPP
#define VISUAL_ACTIONS_HPP

#include "element_monitor.hpp"
#include "memory_monitor.hpp"

#include <dsa/dynamic_array.hpp>
#include <dsa/vector.hpp>

namespace visual
{

class Actions_UI
{
 public:
	void draw();

 private:
	int m_read = 0;
	std::string m_read_value;

	int m_resize = 0;

	int m_write_value = 0;
	int m_write = 0;

	int m_insert_value = 0;
	int m_insert = 0;

	int m_append_value = 0;

	int m_erase = 0;

	dsa::Vector<Element_Monitor<int>, Memory_Monitor> m_vector;

	void properties();
	void accessors();
	void modifiers();

	bool is_in_range(std::size_t index);
	bool is_last_index(std::size_t index);

	void section(const char *label, void (Actions_UI::*interface)());
	void index_input(const char *label, int *value);
	bool button(const char *label, bool enabled);
};

} // namespace visual

#endif
