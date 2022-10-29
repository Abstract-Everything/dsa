#include "memory.hpp"

#include <algorithm>
#include <stdexcept>

namespace visual
{

void Memory::insert(const Memory_Allocation &buffer)
{
	auto it = std::find_if(
	    m_buffers.begin(),
	    m_buffers.end(),
	    [&buffer](const Memory_Allocation &stored)
	    { return Memory_Allocation::overlap(buffer, stored); });

	if (it != m_buffers.end())
	{
		throw std::runtime_error("Tried to add an overlapping buffer");
	}

	m_buffers.push_back(buffer);
}

void Memory::erase(Address address)
{
	auto it = get_by_address(address);
	if (it == m_buffers.end())
	{
		throw std::runtime_error(
		    "Received a deallocate event on a non monitored address");
	}

	m_buffers.erase(it);
}

bool Memory::update_element(Address address, const Memory_Value &value)
{
	auto it = get_by_address(address);
	if (it == m_buffers.end())
	{
		return false;
	}

	it->update_value(address, value);
	return true;
}

Memory::Const_Iterator Memory::begin() const
{
	return m_buffers.begin();
}

Memory::Const_Iterator Memory::end() const
{
	return m_buffers.end();
}

Memory::Iterator Memory::get_by_address(Address address)
{
	return std::find_if(
	    m_buffers.begin(),
	    m_buffers.end(),
	    [&address](const Memory_Allocation &buffer)
	    { return buffer.contains(address); });
}

} // namespace visual
