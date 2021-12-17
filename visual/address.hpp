#ifndef VISUAL_ADDRESS_HPP
#define VISUAL_ADDRESS_HPP

#include <cstdint>

using Address = std::uint64_t;

template<typename T>
Address to_raw_address(T *instance)
{
	return reinterpret_cast<std::uint64_t>(instance);
}

#endif
