#ifndef VISUAL_MEMORY_ELEMENT_HPP
#define VISUAL_MEMORY_ELEMENT_HPP

#include "address.hpp"
#include "memory_value.hpp"

#include <cstddef>
#include <vector>

namespace visual
{

class Memory_Element
{
	using Value          = Memory_Value;
	using Container      = std::vector<Value>;
	using Const_Iterator = Container::const_iterator;

 public:
	Memory_Element(Address address, std::size_t element_size);
	void update_value(Address address, const Memory_Value &value);

	[[nodiscard]] Address address_of_element(std::size_t index) const;

	[[nodiscard]] std::size_t size() const;

	[[nodiscard]] Const_Iterator begin() const;
	[[nodiscard]] Const_Iterator end() const;

	[[nodiscard]] const Value &operator[](std::size_t index) const;

 private:
	Address   m_address;
	Container m_values;

	[[nodiscard]] bool overlaps(
	    Address             lhs_address,
	    const Memory_Value &lhs,
	    Address             rhs_address,
	    const Memory_Value &rhs);
};

} // namespace visual

#endif
