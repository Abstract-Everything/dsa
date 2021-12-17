#ifndef VISUAL_USER_INTERFACE_HPP
#define VISUAL_USER_INTERFACE_HPP

#include "actions.hpp"
#include "element_monitor.hpp"
#include "memory_monitor.hpp"
#include "weak_pointer_monitor.hpp"

#include <dsa/dynamic_array.hpp>
#include <dsa/list.hpp>
#include <dsa/vector.hpp>
#include <dsa/weak_pointer.hpp>

#include <memory>
#include <variant>

namespace visual
{

class User_Interface
{
	using Value  = Element_Monitor<int>;
	using Array  = dsa::Dynamic_Array<Value, Weak_Pointer_Monitor, Memory_Monitor>;
	using Vector = dsa::Vector<Value, Weak_Pointer_Monitor, Memory_Monitor>;
	using List   = dsa::List<Value, Weak_Pointer_Monitor, Memory_Monitor>;

 public:
	void draw();

 private:
	int m_selected_structure;
	std::variant<std::monostate, Actions_UI<Array>, Actions_UI<Vector>, Actions_UI<List>>
	    m_actions;
};

} // namespace visual

#endif
