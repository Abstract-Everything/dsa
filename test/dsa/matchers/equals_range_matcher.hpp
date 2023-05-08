#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>

namespace test
{
// TODO: Use EqualsRange from Catch2 (introduced in 3.3.0)
// https://github.com/catchorg/Catch2/blob/devel/docs/matchers.md

/// @brief Allows use to use Catch2 '_THAT' macros to check that elements of two
/// ranges match element by element
template<typename Range>
struct EqualsRangeMatcher : Catch::Matchers::MatcherGenericBase
{
	explicit EqualsRangeMatcher(Range const &range) : range{range} {
	}

	template<typename OtherRange>
	bool match(OtherRange const &other) const {
		return std::equal(
		    std::begin(range),
		    std::end(range),
		    std::begin(other),
		    std::end(other));
	}

	std::string describe() const override {
		return "Equals: " + Catch::rangeToString(range);
	}

 private:
	Range const &range;
};

template<typename Range>
auto EqualsRange(Range const &range) -> EqualsRangeMatcher<Range> {
	return EqualsRangeMatcher<Range>{range};
}

template<typename T>
auto EqualsRange(std::initializer_list<T> const &range)
    -> EqualsRangeMatcher<std::initializer_list<T>> {
	return EqualsRangeMatcher<std::initializer_list<T>>{range};
}

} // namespace test
