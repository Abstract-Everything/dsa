#include "buffer.hpp"

namespace visual
{

Buffer::Buffer(std::uint64_t address, std::size_t elements_count, std::size_t element_size)
    : m_address(address)
    , m_elements_count(elements_count)
    , m_element_size(element_size)
{
}

std::uint64_t Buffer::address() const
{
	return m_address;
}

bool Buffer::contains(std::uint64_t address) const
{
	return index_of(address) < m_element_size;
}

std::uint64_t Buffer::index_of(std::uint64_t address) const
{
	return (address - m_address) / m_element_size;
}

std::size_t Buffer::count() const
{
	return m_elements_count;
}

bool Buffer::overlap(const Buffer &lhs, const Buffer &rhs)
{
	const Buffer &first  = lhs.address() < rhs.address() ? lhs : rhs;
	const Buffer &second = lhs.address() < rhs.address() ? rhs : lhs;
	return first.contains(second.address());
}

} // namespace visual
