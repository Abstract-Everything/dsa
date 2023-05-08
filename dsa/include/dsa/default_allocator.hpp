#ifndef DSA_DEFAULT_ALLOCATOR_HPP
#define DSA_DEFAULT_ALLOCATOR_HPP

namespace dsa
{

template<typename Value_t>
class Default_Allocator
{
 public:
	using Value = Value_t;

	Default_Allocator() = default;

	template<typename T>
	explicit Default_Allocator(
	    Default_Allocator<T> const & /* allocator */) {
	}
};

} // namespace dsa

#endif
