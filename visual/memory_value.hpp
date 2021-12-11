#ifndef VISUAL_ELEMENT_HPP
#define VISUAL_ELEMENT_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace visual
{

class Memory_Value
{
 public:
	explicit Memory_Value(bool initialized = false, std::string_view value = "");

	void update(bool initialized, std::string_view string);

	[[nodiscard]] std::string_view value() const;
	[[nodiscard]] bool             initialized() const;

 private:
	bool        m_initialized = false;
	std::string m_value       = "?";
};

} // namespace visual

#endif
