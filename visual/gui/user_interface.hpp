#ifndef VISUAL_USER_INTERFACE_HPP
#define VISUAL_USER_INTERFACE_HPP

#include "actions.hpp"
#include "visual_monitor.hpp"

#include <dsa/binary_tree.hpp>
#include <dsa/dynamic_array.hpp>
#include <dsa/heap.hpp>
#include <dsa/list.hpp>
#include <dsa/memory_monitor.hpp>
#include <dsa/vector.hpp>

#include <variant>

namespace visual
{

class User_Interface
{
	using Value = int;

	template<typename T>
	using Allocator = dsa::Memory_Monitor<T, Visual_Monitor>;

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
