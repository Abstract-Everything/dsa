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
	    bool             initialised = false,
	    std::string_view value       = "");

	[[nodiscard]] std::string_view value() const;
	[[nodiscard]] bool             initialised() const;

 private:
	bool        m_initialised = false;
	std::string m_value       = "?";
};

} // namespace visual

#endif
