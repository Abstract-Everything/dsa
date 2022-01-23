#ifndef VISUAL_MEMORY_MONITOR_HPP
#define VISUAL_MEMORY_MONITOR_HPP

#include "address.hpp"
#include "allocated_array_event.hpp"
#include "deallocated_array_event.hpp"
#include "event.hpp"
#include "templates.hpp"
#include "weak_pointer_monitor.hpp"

#include <dsa/allocator_traits.hpp>
#include <dsa/default_allocator.hpp>

DEFINE_HAS_MEMBER(uninitialise);

namespace visual
{

template<typename Value_t>
class Memory_Monitor
{
 public:
	using Value         = Value_t;
	using Pointer       = Weak_Pointer_Monitor<Value>;
	using Const_Pointer = Weak_Pointer_Monitor<const Value>;

 private:
	using Allocator    = dsa::Default_Allocator<Value>;
	using Alloc_Traits = dsa::Allocator_Traits<Allocator>;

 public:

	Memory_Monitor() = default;

	template<typename T>
	explicit Memory_Monitor(Memory_Monitor<T> const& /* monitor */)
	{
	}

	[[nodiscard]] Pointer allocate(std::size_t size) const
	{
		Allocator allocator;
		Pointer   pointer = Alloc_Traits::allocate(allocator, size);

		visual::Dispatch(Allocated_Array_Event{
		    to_raw_address(pointer.get()),
		    sizeof(Value),
		    size});

		for (std::size_t i = 0; i < size; ++i)
		{
			Alloc_Traits::construct(
			    allocator,
			    (pointer + i).get(),
			    Value());

			if constexpr (has_member_uninitialise_v<Value>)
			{
				pointer[i].uninitialise();
			}
		}

		return pointer;
	}

	void deallocate(Pointer pointer, std::size_t size)
	{
		if (pointer == nullptr)
		{
			return;
		}

		visual::Dispatch(
		    Deallocated_Array_Event{to_raw_address(pointer.get())});

		Allocator allocator;
		Alloc_Traits::deallocate(allocator, pointer.get(), size);
	}

	template<typename... Arguments>
	void construct(Pointer pointer, Arguments &&...arguments)
	{
		Allocator allocator;
		Alloc_Traits::construct(
		    allocator,
		    pointer.get(),
		    std::forward<Arguments>(arguments)...);
	}

	void destroy(Pointer pointer)
	{
		Allocator allocator;
		Alloc_Traits::destroy(allocator, pointer.get());
	}
};

} // namespace visual
#endif
