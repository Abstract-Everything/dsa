#ifndef DSA_ALLOCATOR_TRAITS_HPP
#define DSA_ALLOCATOR_TRAITS_HPP

#include <dsa/type_detector.hpp>

#include <cstddef>
#include <memory>
#include <type_traits>

namespace dsa
{

namespace detail
{

class Allocator_Traits_Base
{
 protected:
	template<typename Allocator>
	using Has_Reference = typename Allocator::Reference;

	template<typename Default_Type, typename Allocator>
	using Detect_Reference_T = Detect_Default_T<Default_Type, Has_Reference, Allocator>;

	template<typename Allocator>
	using Has_Const_Reference = typename Allocator::Const_Reference;

	template<typename Default_Type, typename Allocator>
	using Detect_Const_Reference_T =
	    Detect_Default_T<Default_Type, Has_Const_Reference, Allocator>;

	template<typename Allocator>
	using Has_Pointer = typename Allocator::Pointer;

	template<typename Default_Type, typename Allocator>
	using Detect_Pointer_T = Detect_Default_T<Default_Type, Has_Pointer, Allocator>;

	template<typename Allocator>
	using Has_Const_Pointer = typename Allocator::Const_Pointer;

	template<typename Default_Type, typename Allocator>
	using Detect_Const_Pointer_T = Detect_Default_T<Default_Type, Has_Const_Pointer, Allocator>;

	template<typename Allocator>
	using Has_Allocate_Operator = decltype(std::declval<Allocator>().allocate(std::size_t()));

	template<typename Allocator>
	static constexpr bool has_allocate() {
		return Detect_V<Has_Allocate_Operator, Allocator>;
	}

	template<typename Allocator, typename Pointer, typename... Arguments>
	using Has_Construct_Operator = decltype(std::declval<Allocator &&>().construct(
	    std::declval<Pointer &&>(),
	    std::declval<Arguments &&>()...));

	template<typename Allocator, typename Pointer, typename... Arguments>
	static constexpr bool has_construct() {
		return Detect_V<Has_Construct_Operator, Allocator, Pointer, Arguments...>;
	}

	template<typename Allocator, typename Pointer>
	using Has_Deallocate_Operator =
	    decltype(std::declval<Allocator &&>().deallocate(std::declval<Pointer &&>(), std::size_t()));

	template<typename Allocator, typename Pointer>
	static constexpr bool has_deallocate() {
		return Detect_V<Has_Deallocate_Operator, Allocator, Pointer>;
	}

	template<typename Allocator, typename Pointer>
	using Has_Destroy_Operator =
	    decltype(std::declval<Allocator>().destroy(std::declval<Pointer &&>()));

	template<typename Allocator, typename Pointer>
	static constexpr bool has_destroy() {
		return Detect_V<Has_Destroy_Operator, Allocator, Pointer>;
	}
};

} // namespace detail

template<typename Allocator_t>
class Allocator_Traits : detail::Allocator_Traits_Base
{
 public:
	using Allocator       = Allocator_t;
	using Value           = typename Allocator::Value;
	using Reference       = Detect_Reference_T<Value &, Allocator>;
	using Const_Reference = Detect_Const_Reference_T<Value const &, Allocator>;
	using Pointer         = Detect_Pointer_T<Value *, Allocator>;
	using Const_Pointer   = Detect_Const_Pointer_T<Value const *, Allocator>;

 private:
	using Std_Allocator        = std::allocator<Value>;
	using Std_Allocator_Traits = std::allocator_traits<Std_Allocator>;

 public:
	static constexpr Allocator propogate_or_create_instance(Allocator const &allocator) {
		if constexpr (std::is_copy_constructible_v<Allocator>)
		{
			return Allocator(allocator);
		}
		return Allocator();
	}

	static constexpr Pointer allocate(Allocator &allocator, std::size_t count) {
		if constexpr (has_allocate<Allocator>())
		{
			return allocator.allocate(count);
		}
		else
		{
			Std_Allocator std_allocator;
			return Std_Allocator_Traits::allocate(std_allocator, count);
		}
	}

	template<typename Pointer_t, typename... Arguments>
	static constexpr void construct(
	    // MSVC gives unused error if both branches are not used
	    [[maybe_unused]] Allocator &allocator,
	    Pointer_t                 &&pointer,
	    Arguments &&...arguments) {
		static_assert(
		    std::is_same_v<Pointer, std::remove_cvref_t<Pointer_t>>,
		    "These traits should not be passed a different pointer "
		    "type");

		if constexpr (has_construct<Allocator, Pointer, Arguments...>())
		{
			allocator.construct(
			    std::forward<Pointer_t>(pointer),
			    std::forward<Arguments>(arguments)...);
		}
		else
		{
			Std_Allocator std_allocator;
			Std_Allocator_Traits::construct(
			    std_allocator,
			    std::forward<Pointer_t>(pointer),
			    std::forward<Arguments>(arguments)...);
		}
	}

	template<typename Pointer_t>
	static constexpr void deallocate(Allocator &allocator, Pointer_t &&pointer, std::size_t count) {
		static_assert(
		    std::is_same_v<Pointer, std::remove_cvref_t<Pointer_t>>,
		    "These traits should not be passed a different pointer "
		    "type");

		if constexpr (has_deallocate<Allocator, Pointer>())
		{
			allocator.deallocate(std::forward<Pointer_t>(pointer), count);
		}
		else
		{
			Std_Allocator std_allocator;
			Std_Allocator_Traits::deallocate(
			    std_allocator,
			    std::forward<Pointer_t>(pointer),
			    count);
		}
	}

	template<typename Pointer_t>
	static constexpr void destroy(
	    // MSVC gives unused error if both branches are not used
	    [[maybe_unused]] Allocator &allocator,
	    Pointer_t                 &&pointer) {
		if constexpr (has_destroy<Allocator, Pointer>())
		{
			allocator.destroy(std::forward<Pointer_t>(pointer));
		}
		else
		{
			Std_Allocator std_allocator;
			Std_Allocator_Traits::destroy(std_allocator, std::forward<Pointer_t>(pointer));
		}
	}
};

} // namespace dsa

#endif
