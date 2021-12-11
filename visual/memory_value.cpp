#include "memory_value.hpp"

namespace visual
{

Memory_Value::Memory_Value(bool initialized, std::string_view value)
{
	update(initialized, value);
}

void Memory_Value::update(bool initialized, std::string_view string)
{
	m_initialized = initialized;
	if (initialized)
	{
		m_value = string;
	}
	else
	{
		m_value = "?";
	}
}

std::string_view Memory_Value::value() const
{
	return m_value;
}

bool Memory_Value::initialized() const
{
	return m_initialized;
}

} // namespace visual
