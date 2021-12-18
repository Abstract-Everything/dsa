#ifndef VISUAL_MEMORY_HPP
#define VISUAL_MEMORY_HPP

#include "memory_allocation.hpp"

#include <vector>

namespace visual
{

class Memory
{
	using Container      = std::vector<Memory_Allocation>;
	using Const_Iterator = Container::const_iterator;
	using Iterator       = Container::iterator;

 public:
	void insert(const Memory_Allocation &buffer);
	void erase(Address address);
	bool update_element(Address address, const Memory_Value &value);

	[[nodiscard]] Const_Iterator begin() const;
	[[nodiscard]] Const_Iterator end() const;

 private:
	Container m_buffers;

	Iterator get_by_address(Address address);
};

} // namespace visual

#endif
