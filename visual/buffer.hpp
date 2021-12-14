#ifndef VISUAL_BUFFER_HPP
#define VISUAL_BUFFER_HPP

#include "address.hpp"
#include "memory_value.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace visual
{

class Buffer
{
	using Container      = std::vector<Memory_Value>;
	using Iterator       = Container::iterator;
	using Const_Iterator = Container::const_iterator;

 public:
	Buffer(Address address, std::size_t elements_count, std::size_t element_size);

	[[nodiscard]] Address     address() const;
	[[nodiscard]] bool        contains(Address address) const;
	[[nodiscard]] std::size_t index_of(Address address) const;

	[[nodiscard]] std::size_t size() const;

	[[nodiscard]] Iterator       begin();
	[[nodiscard]] Const_Iterator begin() const;

	[[nodiscard]] Iterator       end();
	[[nodiscard]] Const_Iterator end() const;

	[[nodiscard]] static bool overlap(const Buffer &lhs, const Buffer &rhs);

	[[nodiscard]] bool update_value(Address address, const Memory_Value &value);

 private:
	Address     m_address;
	std::size_t m_element_size;

	Container m_elements;
};

} // namespace visual

#endif
