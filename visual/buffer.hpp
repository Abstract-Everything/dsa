#ifndef VISUAL_BUFFER_HPP
#define VISUAL_BUFFER_HPP

#include <cstdint>
#include <cstddef>

namespace visual
{

class Buffer
{
 public:
	Buffer(
	    std::uint64_t address,
	    std::size_t   elements_count,
	    std::size_t   element_size);

	[[nodiscard]] std::uint64_t address() const;
	[[nodiscard]] bool          contains(std::uint64_t address) const;
	[[nodiscard]] std::uint64_t index_of(std::uint64_t address) const;

	[[nodiscard]] std::size_t count() const;

	[[nodiscard]] static bool overlap(const Buffer &lhs, const Buffer &rhs);

 private:
	std::uint64_t m_address;
	std::size_t   m_elements_count;
	std::size_t   m_element_size;
};

} // namespace visual

#endif
