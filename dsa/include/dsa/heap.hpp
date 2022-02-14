#ifndef DSA_HEAP_HPP
#define DSA_HEAP_HPP

#include <dsa/allocator_traits.hpp>
#include <dsa/default_allocator.hpp>

namespace dsa
{

/**
 * @brief Represents a set of contiguous elements of the same type organised in
 * such a way as to allow fast access to the greatest element.
 *
 * @ingroup containers
 *
 * @tparam Value_t: The type of element to store
 * @tparam Allocator_Base: The type of allocator used for memory management
 *
 */
template<typename Value_t, template<typename> typename Allocator_Base = Default_Allocator>
class Heap
{
 private:
	using Alloc_Traits = Allocator_Traits<Allocator_Base<Value_t>>;

 public:
	using Allocator     = typename Alloc_Traits::Allocator;
	using Value         = typename Alloc_Traits::Value;
	using Pointer       = typename Alloc_Traits::Pointer;
	using Const_Pointer = typename Alloc_Traits::Const_Pointer;

};

} // namespace dsa

#endif
