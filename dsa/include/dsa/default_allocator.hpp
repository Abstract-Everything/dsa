#ifndef DSA_DEFAULT_ALLOCATOR_HPP
#define DSA_DEFAULT_ALLOCATOR_HPP

namespace dsa
{

template<typename Value_t>
class Default_Allocator
{
 public:
	using Value         = Value_t;
	using Pointer       = Value *;
	using Const_Pointer = Value const *;
};

} // namespace dsa

#endif
