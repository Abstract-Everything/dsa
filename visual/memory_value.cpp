#include "memory_value.hpp"

namespace visual
{

Memory_Value::Memory_Value(std::size_t size, bool initialised, std::string_view value)
    : m_size(size)
    , m_initialised(initialised)
    , m_value(initialised ? value : "?")
{
}

std::size_t Memory_Value::size() const
{
	return m_size;
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
