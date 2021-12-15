#ifndef VISUAL_ALLOCATED_ARRAY_EVENT_HPP
#define VISUAL_ALLOCATED_ARRAY_EVENT_HPP

#include "address.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

namespace visual
{

class Allocated_Array_Event
{
 public:
	Allocated_Array_Event(
	    Address     address,
	    std::size_t element_size,
	    std::size_t size);

	[[nodiscard]] Address     address() const;
	[[nodiscard]] std::size_t element_size() const;
	[[nodiscard]] std::size_t size() const;

	[[nodiscard]] std::string to_string() const;

 private:
	Address     m_address;
	std::size_t m_element_size;
	std::size_t m_size;
};

} // namespace visual

#endif
