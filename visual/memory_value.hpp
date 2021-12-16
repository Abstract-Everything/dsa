#ifndef VISUAL_MEMORY_VALUE_HPP
#define VISUAL_MEMORY_VALUE_HPP

#include "address.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace visual
{

class Memory_Value
{
 public:
	explicit Memory_Value(
	    std::size_t      size,
	    bool             initialised = false,
	    std::string_view value       = "");

	explicit Memory_Value(std::size_t size, bool initialised, Address address);

	[[nodiscard]] std::size_t size() const;
	[[nodiscard]] bool        initialised() const;

	[[nodiscard]] bool    is_pointer() const;
	[[nodiscard]] Address pointee_address() const;

	[[nodiscard]] std::string value() const;

 private:
	std::size_t                        m_size        = 0U;
	bool                               m_initialised = false;
	std::variant<Address, std::string> m_value       = "?";
};

} // namespace visual

#endif
