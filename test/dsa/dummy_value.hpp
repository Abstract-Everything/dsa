#ifndef TEST_DSA_STATIC_DUMMY_VALUE_HPP
#define TEST_DSA_STATIC_DUMMY_VALUE_HPP

namespace test
{

struct Dummy_Value_Construct_Tag
{};

class Dummy_Value
{
 public:
	constexpr explicit Dummy_Value(Dummy_Value_Construct_Tag /* tag */) {
	}

	~Dummy_Value()                              = default;
	Dummy_Value(Dummy_Value const &)            = delete;
	Dummy_Value(Dummy_Value &&)                 = delete;
	Dummy_Value &operator=(Dummy_Value const &) = delete;
	Dummy_Value &operator=(Dummy_Value &&)      = delete;
};

} // namespace test

#endif
