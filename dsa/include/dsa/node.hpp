#ifndef DSA_NODE_HPP
#define DSA_NODE_HPP

#include <dsa/allocator_traits.hpp>

namespace dsa::detail
{

template<typename Node_t>
class Node_Traits
{
 private:
	using Alloc_Traits = typename Node_t::Alloc_Traits;

	using Satellite_Alloc_Traits = typename Node_t::Satellite_Alloc_Traits;
	using Satellite_Allocator = typename Satellite_Alloc_Traits::Allocator;
	using Satellite_Pointer   = typename Satellite_Alloc_Traits::Pointer;

 public:
	using Allocator     = typename Alloc_Traits::Allocator;
	using Pointer       = typename Alloc_Traits::Pointer;
	using Const_Pointer = typename Alloc_Traits::Const_Pointer;

	/**
	 * @brief Allocates a node and constructs the held satellite.
	 */
	template<typename... Arguments>
	[[nodiscard]] static Pointer create_node(
	    Allocator allocator,
	    Arguments &&...arguments) {
		Pointer pointer = Alloc_Traits::allocate(allocator, 1);
		pointer->initialise();

		Satellite_Allocator satellite_allocator(allocator);
		Satellite_Alloc_Traits::construct(
		    satellite_allocator,
		    Satellite_Pointer(&pointer->m_satellite),
		    std::forward<Arguments>(arguments)...);

		return pointer;
	}

	/**
	 * @brief Destroys the satellite of the node and deallocates its
	 * storage. This function does not recursively destroy nodes.
	 */
	static void destroy_node(Allocator allocator, Pointer node) {
		Satellite_Allocator satellite_allocator(allocator);
		Satellite_Alloc_Traits::destroy(
		    satellite_allocator,
		    &node->m_satellite);
		Alloc_Traits::deallocate(allocator, node, 1);
	}

	Node_Traits()                               = delete;
	~Node_Traits()                              = delete;
	Node_Traits(Node_Traits const &)            = delete;
	Node_Traits(Node_Traits &&)                 = delete;
	Node_Traits &operator=(Node_Traits const &) = delete;
	Node_Traits &operator=(Node_Traits &&)      = delete;
};

} // namespace dsa::detail

#endif
