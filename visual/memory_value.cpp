#include "memory_value.hpp"

#include <fmt/format.h>

#include <cassert>

namespace visual
{

Memory_Value::Memory_Value(std::size_t size, bool initialised, std::string_view value)
    : m_size(size)
    , m_initialised(initialised)
    , m_value(initialised ? std::string{ value } : "?")
{
}

Memory_Value::Memory_Value(std::size_t size, bool initialised, Address address)
    : m_size(size)
    , m_initialised(initialised)
    , m_value(initialised ? address : 0U)
{
}

std::size_t Memory_Value::size() const
{
	return m_size;
}

bool Memory_Value::initialised() const
{
	return m_initialised;
}

bool Memory_Value::is_pointer() const
{
	return std::get_if<Address>(&m_value) != nullptr;
}

Address Memory_Value::pointee_address() const
{
	if (is_pointer())
	{
		return std::get<Address>(m_value);
	}

	assert(!"Tried retreiving a value as a pointer");
	return 0U;
}

std::string Memory_Value::value() const
{
	if (is_pointer())
	{
		return fmt::format("{0:#x}", std::get<Address>(m_value));
	}

	return std::get<std::string>(m_value);
}

} // namespace visual
