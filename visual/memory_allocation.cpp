#include "memory_allocation.hpp"

#include <algorithm>
#include <cassert>

namespace visual
{

Memory_Allocation::Memory_Allocation(
    Address     address,
    std::size_t elements_count,
    std::size_t element_size)
    : m_address(address)
    , m_element_size(element_size)
{
	for (std::size_t i = 0; i < elements_count; ++i)
	{
		m_elements.push_back(
		    Memory_Element{address + i * element_size, element_size});
	}
}

Address Memory_Allocation::address() const
{
	return m_address;
}

bool Memory_Allocation::contains(Address address) const
{
	return m_address == address || index_of(address) < m_elements.size();
}

std::size_t Memory_Allocation::index_of(Address address) const
{
	return (address - m_address) / m_element_size;
}

std::size_t Memory_Allocation::size() const
{
	return m_elements.size();
}

std::size_t Memory_Allocation::element_size() const
{
	return m_element_size;
}

std::size_t Memory_Allocation::max_element_size() const
{
	auto it = std::max_element(
	    m_elements.begin(),
	    m_elements.end(),
	    [](const visual::Memory_Element &lhs, const visual::Memory_Element &rhs)
	    { return lhs.size() < rhs.size(); });
	return it == m_elements.end() ? 0 : it->size();
}

Memory_Allocation::Iterator Memory_Allocation::begin()
{
	return m_elements.begin();
}

Memory_Allocation::Const_Iterator Memory_Allocation::begin() const
{
	return m_elements.cbegin();
}

Memory_Allocation::Iterator Memory_Allocation::end()
{
	return m_elements.end();
}

Memory_Allocation::Const_Iterator Memory_Allocation::end() const
{
	return m_elements.cend();
}

bool Memory_Allocation::overlap(
    const Memory_Allocation &lhs,
    const Memory_Allocation &rhs)
{
	auto const &first  = lhs.address() < rhs.address() ? lhs : rhs;
	auto const &second = lhs.address() < rhs.address() ? rhs : lhs;
	return first.contains(second.address());
}

void Memory_Allocation::update_value(Address address, const Memory_Value &value)
{
	assert(
	    contains(address) && "Tried updating a value outside of the buffer");
	m_elements[index_of(address)].update_value(address, value);
}

} // namespace visual
