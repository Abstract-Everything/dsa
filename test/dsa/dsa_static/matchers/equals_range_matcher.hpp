#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>

namespace test
{

/// @brief Allows use to use Catch2 '_THAT' macros to check that elements of two
/// ranges match element by element
template<typename Range>
struct EqualsRangeMatcher : Catch::Matchers::MatcherGenericBase
{
	explicit EqualsRangeMatcher(Range const &range) : range{range}
	{
	}

	template<typename OtherRange>
	bool match(OtherRange const &other) const
	{
		return std::equal(
		    std::begin(range),
		    std::end(range),
		    std::begin(other),
		    std::end(other));
	}

	std::string describe() const override
	{
		return "Equals: " + Catch::rangeToString(range);
	}

 private:
	Range const &range;
};

template<typename Range>
auto EqualsRange(const Range &range) -> EqualsRangeMatcher<Range>
{
	return EqualsRangeMatcher<Range>{range};
}

} // namespace test
