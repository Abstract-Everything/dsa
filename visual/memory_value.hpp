#ifndef VISUAL_MEMORY_VALUE_HPP
#define VISUAL_MEMORY_VALUE_HPP

#include <cstddef>
#include <cstdint>
#include <string>
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

	[[nodiscard]] std::size_t      size() const;
	[[nodiscard]] std::string_view value() const;
	[[nodiscard]] bool             initialised() const;

 private:
	std::size_t m_size        = 0U;
	bool        m_initialised = false;
	std::string m_value       = "?";
};

} // namespace visual

#endif
