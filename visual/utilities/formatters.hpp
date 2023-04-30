#ifndef VISUAL_UTILITIES_FORMATTERS_HPP
#define VISUAL_UTILITIES_FORMATTERS_HPP

#include <dsa/memory_monitor.hpp>

#include <fmt/ostream.h>

template<typename T>
struct fmt::formatter<dsa::Allocation_Event<T>> : ostream_formatter
{
};

template<typename T>
struct fmt::formatter<dsa::Object_Event<T>> : ostream_formatter
{
};

#endif
