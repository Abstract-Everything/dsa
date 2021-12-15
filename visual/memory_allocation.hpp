#ifndef VISUAL_MEMORY_ALLOCATION_HPP
#define VISUAL_MEMORY_ALLOCATION_HPP

#include "address.hpp"
#include "memory_value.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace visual
{

class Memory_Allocation
{
	using Container      = std::vector<Memory_Value>;
	using Iterator       = Container::iterator;
	using Const_Iterator = Container::const_iterator;

 public:
	Memory_Allocation(Address address, std::size_t elements_count, std::size_t element_size);

	[[nodiscard]] Address     address() const;
	[[nodiscard]] bool        contains(Address address) const;
	[[nodiscard]] std::size_t index_of(Address address) const;

	[[nodiscard]] std::size_t size() const;

	[[nodiscard]] Iterator       begin();
	[[nodiscard]] Const_Iterator begin() const;

	[[nodiscard]] Iterator       end();
	[[nodiscard]] Const_Iterator end() const;

	[[nodiscard]] static bool overlap(const Memory_Allocation &lhs, const Memory_Allocation &rhs);

	void update_value(Address address, const Memory_Value &value);

 private:
	Address     m_address;
	std::size_t m_element_size;

	Container m_elements;
};

} // namespace visual

#endif
