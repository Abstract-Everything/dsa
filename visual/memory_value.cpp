#include "memory_value.hpp"

namespace visual
{

Memory_Value::Memory_Value(bool initialised, std::string_view value)
    : m_initialised(initialised)
    , m_value(initialised ? value : "?")
{
}

std::string_view Memory_Value::value() const
{
	return m_value;
}

bool Memory_Value::initialised() const
{
	return m_initialised;
}

} // namespace visual
