#include "memory_element.hpp"

#include <cassert>
#include <numeric>

namespace visual
{

Memory_Element::Memory_Element(Address address, std::size_t element_size)
    : m_address(address)
{
	m_values.push_back(Memory_Value{ element_size });
}

void Memory_Element::update_value(Address address, const Memory_Value &value)
{
	Address old_address = m_address;
	auto    it          = m_values.begin();
	for (; it != m_values.end() && !overlaps(old_address, *it, address, value);
	     old_address += it->size(), ++it)
	{
	}

	assert(it != m_values.end());
	Memory_Value old_value = *it;
	*it                    = value;

	if (old_address < address)
	{
		it = m_values.insert(it, Memory_Value{ address - old_address });
		it++;
	}

	auto new_address_end = address + value.size();
	auto old_address_end = old_address + old_value.size();
	if (new_address_end < old_address_end)
	{
		m_values.insert(
		    ++it,
		    Memory_Value{ old_address_end - new_address_end });
	}
}

Address Memory_Element::address_of_element(std::size_t index) const
{
	return m_address
	       + std::accumulate(
		   begin(),
		   begin() + static_cast<std::ptrdiff_t>(index),
		   0ULL,
		   [](std::size_t accumulated, const visual::Memory_Value &value)
		   { return accumulated + value.size(); });
}

std::size_t Memory_Element::size() const
{
	return m_values.size();
}

Memory_Element::Const_Iterator Memory_Element::begin() const
{
	return m_values.begin();
}

Memory_Element::Const_Iterator Memory_Element::end() const
{
	return m_values.end();
}

const Memory_Element::Value &Memory_Element::operator[](std::size_t index) const
{
	return m_values[index];
}

bool Memory_Element::overlaps(
    Address             lhs_address,
    const Memory_Value &lhs,
    Address             rhs_address,
    const Memory_Value &rhs)
{
	const bool  is_lhs_smaller = lhs_address < rhs_address;
	auto const &first          = is_lhs_smaller ? lhs : rhs;
	auto const &first_address  = is_lhs_smaller ? lhs_address : rhs_address;
	auto const &second_address = is_lhs_smaller ? rhs_address : lhs_address;
	return first_address == second_address
	       || first_address + first.size() > second_address;
}

} // namespace visual
