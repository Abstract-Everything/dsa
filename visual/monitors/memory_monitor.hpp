#ifndef VISUAL_MEMORY_MONITOR_HPP
#define VISUAL_MEMORY_MONITOR_HPP

#include "allocated_array_event.hpp"

#include <cstdint>

namespace visual
{
template<typename T>
class Memory_Monitor
{
 public:
	[[nodiscard]] T *allocate(std::size_t size) const
	{
		void *pointer = ::operator new(size * sizeof(T));

		visual::Event::Dispatch(std::make_unique<Allocated_Array_Event>(
		    reinterpret_cast<std::uint64_t>(pointer),
		    sizeof(T),
		    size));

		return reinterpret_cast<T *>(pointer);
	}

	void deallocate(T *pointer) const
	{
		::operator delete(pointer);
	}
};

} // namespace visual
#endif
