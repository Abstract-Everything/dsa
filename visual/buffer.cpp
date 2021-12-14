#include "buffer.hpp"

namespace visual
{

Buffer::Buffer(Address address, std::size_t elements_count, std::size_t element_size)
    : m_address(address)
    , m_element_size(element_size)
{
	m_elements.resize(elements_count);
}

Address Buffer::address() const
{
	return m_address;
}

bool Buffer::contains(Address address) const
{
	return index_of(address) < m_elements.size();
}

std::size_t Buffer::index_of(Address address) const
{
	return (address - m_address) / m_element_size;
}

std::size_t Buffer::size() const
{
	return m_elements.size();
}

Buffer::Iterator Buffer::begin()
{
	return m_elements.begin();
}

Buffer::Const_Iterator Buffer::begin() const
{
	return m_elements.cbegin();
}

Buffer::Iterator Buffer::end()
{
	return m_elements.end();
}

Buffer::Const_Iterator Buffer::end() const
{
	return m_elements.cend();
}

bool Buffer::overlap(const Buffer &lhs, const Buffer &rhs)
{
	const Buffer &first  = lhs.address() < rhs.address() ? lhs : rhs;
	const Buffer &second = lhs.address() < rhs.address() ? rhs : lhs;
	return first.contains(second.address());
}

bool Buffer::update_value(Address address, const Memory_Value& value)
{
	if (!contains(address))
	{
		return false;
	}

	m_elements[index_of(address)] = value;
	return true;
}

} // namespace visual
