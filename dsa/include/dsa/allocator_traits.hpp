#ifndef DSA_ALLOCATOR_TRAITS_HPP
#define DSA_ALLOCATOR_TRAITS_HPP

#include <dsa/type_detector.hpp>

#include <cstddef>
#include <memory>
#include <type_traits>

namespace dsa
{

template<typename Allocator_t>
class Allocator_Traits
{
 public:
	using Allocator     = Allocator_t;
	using Value         = typename Allocator::Value;
	using Pointer       = typename Allocator::Pointer;
	using Const_Pointer = typename Allocator::Const_Pointer;

 private:
	using Std_Allocator        = std::allocator<Value>;
	using Std_Allocator_Traits = std::allocator_traits<Std_Allocator>;

	template<typename Allocator>
	using Has_Allocate_Operator =
	    decltype(std::declval<Allocator>().allocate(std::size_t()));

	template<typename Allocator, typename Pointer, typename... Arguments>
	using Has_Construct_Operator =
	    decltype(std::declval<Allocator>().construct(
		Pointer(),
		std::declval<Arguments>()...));

	template<typename Allocator>
	using Has_Deallocate_Operator =
	    decltype(std::declval<Allocator>().deallocate(Pointer(), std::size_t()));

	template<typename Allocator, typename Pointer>
	using Has_Destroy_Operator =
	    decltype(std::declval<Allocator>().construct(Pointer()));

	static constexpr bool has_allocate()
	{
		return Detect_V<Has_Allocate_Operator, Allocator>;
	}

	template<typename... Arguments>
	static constexpr bool has_construct()
	{
		return Detect_V<Has_Construct_Operator, Allocator, Pointer, Arguments...>;
	}

	static constexpr bool has_deallocate()
	{
		return Detect_V<Has_Deallocate_Operator, Allocator>;
	}

	static constexpr bool has_destroy()
	{
		return Detect_V<Has_Destroy_Operator, Allocator, Pointer>;
	}

 public:
	static Pointer allocate(Allocator allocator, std::size_t count)
	{
		if constexpr (has_allocate())
		{
			return allocator.allocate(count);
		}
		else
		{
			Std_Allocator std_allocator;
			return Std_Allocator_Traits::allocate(std_allocator, count);
		}
	}

	template<typename... Arguments>
	static void construct(
	    // MSVC gives unused error if both branches are not used
	    [[maybe_unused]] Allocator allocator,
	    Pointer                    pointer,
	    Arguments &&...arguments)
	{
		if constexpr (has_construct<Arguments...>())
		{
			allocator.construct(
			    pointer,
			    std::forward<Arguments>(arguments)...);
		}
		else
		{
			// ToDo: Use construct_at
			Std_Allocator std_allocator;
			Std_Allocator_Traits::construct(
			    std_allocator,
			    pointer,
			    std::forward<Arguments>(arguments)...);
		}
	}

	static void deallocate(Allocator allocator, Pointer pointer, std::size_t count)
	{
		if constexpr (has_deallocate())
		{
			allocator.deallocate(pointer, count);
		}
		else
		{
			Std_Allocator std_allocator;
			Std_Allocator_Traits::deallocate(
			    std_allocator,
			    pointer,
			    count);
		}
	}

	static void destroy(
	    // MSVC gives unused error if both branches are not used
	    [[maybe_unused]] Allocator allocator,
	    Pointer                    pointer)
	{
		if constexpr (has_destroy())
		{
			allocator.destroy(pointer);
		}
		else
		{
			// ToDo: Use destroy_at
			Std_Allocator std_allocator;
			Std_Allocator_Traits::destroy(std_allocator, pointer);
		}
	}
};

} // namespace dsa

#endif
