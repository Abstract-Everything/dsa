#ifndef VISUAL_MEMORY_MONITOR_HPP
#define VISUAL_MEMORY_MONITOR_HPP

#include "address.hpp"
#include "allocated_array_event.hpp"
#include "deallocated_array_event.hpp"
#include "event.hpp"
#include "templates.hpp"

DEFINE_HAS_MEMBER(uninitialise);

namespace visual
{
template<typename T>
class Memory_Monitor
{
 public:
	[[nodiscard]] T *allocate(std::size_t size) const
	{
		void *pointer = ::operator new(size * sizeof(T));

		visual::Dispatch(
		    Allocated_Array_Event{to_raw_address(pointer), sizeof(T), size});

		T *typed_pointer = reinterpret_cast<T *>(pointer);
		for (std::size_t i = 0; i < size; ++i)
		{
			new (typed_pointer + i) T{};
			if constexpr (has_member_uninitialise_v<T>)
			{
				typed_pointer[i].uninitialise();
			}
		}

		return typed_pointer;
	}

	void deallocate(T *pointer, std::size_t /* size */) const
	{
		if (pointer == nullptr)
		{
			return;
		}

		visual::Dispatch(Deallocated_Array_Event{to_raw_address(pointer)});

		::operator delete(pointer);
	}
};

} // namespace visual
#endif
