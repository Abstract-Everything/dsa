#ifndef VISUAL_USER_INTERFACE_HPP
#define VISUAL_USER_INTERFACE_HPP

#include "actions.hpp"
#include "element_monitor.hpp"
#include "memory_monitor.hpp"
#include "weak_pointer_monitor.hpp"

#include <dsa/binary_tree.hpp>
#include <dsa/dynamic_array.hpp>
#include <dsa/list.hpp>
#include <dsa/vector.hpp>
#include <dsa/heap.hpp>

#include <variant>

namespace visual
{

class User_Interface
{
	using Value = Element_Monitor<int>;

	template<typename T>
	using Allocator = Memory_Monitor<T>;

	using Array       = dsa::Dynamic_Array<Value, Allocator>;
	using Vector      = dsa::Vector<Value, Allocator>;
	using List        = dsa::List<Value, Allocator>;
	using Binary_Tree = dsa::Binary_Tree<Value, Allocator>;
	using Heap        = dsa::Heap<Value, decltype(std::less{}), Allocator>;

	using Structures_Actions = std::variant<
	    std::monostate,
	    Actions_UI<Array>,
	    Actions_UI<Vector>,
	    Actions_UI<List>,
	    Actions_UI<Binary_Tree>,
	    Actions_UI<Heap>>;

 public:
	void draw();

 private:
	int                m_selected_structure;
	Structures_Actions m_actions;
};

} // namespace visual

#endif
