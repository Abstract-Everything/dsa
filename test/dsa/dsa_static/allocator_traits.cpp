#include <dsa/allocator_traits.hpp>
#include <dsa/default_allocator.hpp>

#include "empty_value.hpp"

#include <catch2/catch.hpp>

TEST_CASE("Created test file", "[allocator_traits]")
{
	using Allocator = dsa::Default_Allocator<Empty_Value>;
	using Traits    = dsa::Allocator_Traits<Allocator>;

	STATIC_REQUIRE(std::is_same_v<Traits::Allocator, Allocator>);
	STATIC_REQUIRE(std::is_same_v<Traits::Value, Empty_Value>);
	STATIC_REQUIRE(std::is_same_v<Traits::Reference, Empty_Value &>);
	STATIC_REQUIRE(std::is_same_v<Traits::Const_Reference, Empty_Value const &>);
	STATIC_REQUIRE(std::is_same_v<Traits::Pointer, Empty_Value *>);
	STATIC_REQUIRE(std::is_same_v<Traits::Const_Pointer, Empty_Value const *>);
}
