#ifndef DSA_DEFAULT_ALLOCATOR_HPP
#define DSA_DEFAULT_ALLOCATOR_HPP

namespace dsa
{

template<typename Value_t>
class Default_Allocator
{
 public:
	using Value           = Value_t;
	using Reference       = Value &;
	using Const_Reference = Value const &;
	using Pointer         = Value *;
	using Const_Pointer   = Value const *;

	Default_Allocator() = default;

	template<typename T>
	explicit Default_Allocator(Default_Allocator<T> const & /* allocator */)
	{
	}
};

} // namespace dsa

#endif
