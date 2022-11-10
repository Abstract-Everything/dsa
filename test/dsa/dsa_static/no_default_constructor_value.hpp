#ifndef TEST_DSA_STATIC_NO_DEFAULT_CONSTRUCTOR_VALUE_HPP
#define TEST_DSA_STATIC_NO_DEFAULT_CONSTRUCTOR_VALUE_HPP

namespace test
{

struct No_Default_Constructor_Value_Construct_Tag
{
};

/// This class can only be constructed from the accompanying tag. This allows us
/// to test that the object was constructed using the provided arguments and not
/// using a default constructor.
class No_Default_Constructor_Value
{
 public:
	constexpr explicit No_Default_Constructor_Value() = delete;
	constexpr explicit No_Default_Constructor_Value(
	    No_Default_Constructor_Value_Construct_Tag /* tag */)
	{
	}

	constexpr ~No_Default_Constructor_Value() = default;

	constexpr No_Default_Constructor_Value(
	    No_Default_Constructor_Value const &) = delete;
	constexpr No_Default_Constructor_Value &operator=(
	    No_Default_Constructor_Value const &) = delete;

	constexpr No_Default_Constructor_Value(No_Default_Constructor_Value &&) = delete;
	constexpr No_Default_Constructor_Value &operator=(
	    No_Default_Constructor_Value &&) = delete;
};

} // namespace test

#endif
