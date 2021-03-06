#ifndef VISUAL_DEALLOCATED_ARRAY_EVENT_HPP
#define VISUAL_DEALLOCATED_ARRAY_EVENT_HPP

#include "address.hpp"

#include <string>

namespace visual
{
class Deallocated_Array_Event
{
 public:
	explicit Deallocated_Array_Event(Address address);

	[[nodiscard]] Address address() const;

	[[nodiscard]] std::string to_string() const;

 private:
	Address m_address;
};

} // namespace visual

#endif
