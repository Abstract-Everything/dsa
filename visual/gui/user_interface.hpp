#ifndef VISUAL_USER_INTERFACE_HPP
#define VISUAL_USER_INTERFACE_HPP

#include "element_monitor.hpp"
#include "memory_monitor.hpp"

#include <dsa/dynamic_array.hpp>
#include <dsa/vector.hpp>

#include <variant>
#include "actions.hpp"

namespace visual
{

class User_Interface
{
	using Value  = Element_Monitor<int>;
	using Array  = dsa::Dynamic_Array<Value, Memory_Monitor>;
	using Vector = dsa::Vector<Value, Memory_Monitor>;

 public:
	void draw();

 private:
	int m_selected_structure;
	std::variant<Actions_UI<Array>, Actions_UI<Vector>> m_actions;
};

} // namespace visual

#endif
