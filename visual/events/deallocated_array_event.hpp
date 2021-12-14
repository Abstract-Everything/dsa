#ifndef VISUAL_DEALLOCATED_ARRAY_EVENT_HPP
#define VISUAL_DEALLOCATED_ARRAY_EVENT_HPP


#include <cstdint>

namespace visual
{
class Deallocated_Array_Event
{
 public:
	explicit Deallocated_Array_Event(std::uint64_t address);

	[[nodiscard]] std::uint64_t address() const;

 private:
	std::uint64_t m_address;
};

} // namespace visual

#endif
