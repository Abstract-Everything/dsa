#ifndef TEST_DSA_STATIC_EMPTY_VALUE_HPP
#define TEST_DSA_STATIC_EMPTY_VALUE_HPP

#include <ostream>

struct Empty_Value
{
	friend auto operator<<(std::ostream &stream, Empty_Value const & /*value*/)
	    -> std::ostream &
	{
		return stream << "{}";
	}
};

#endif
