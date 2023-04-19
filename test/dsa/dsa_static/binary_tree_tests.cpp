#include "allocation_verifier.hpp"
#include "equals_range_matcher.hpp"
#include "memory_monitor_handler_scope.hpp"

#include <dsa/binary_tree.hpp>
#include <dsa/memory_monitor.hpp>

#include <catch2/catch_all.hpp>

namespace test
{

template<typename T>
using Allocator_Base = dsa::Memory_Monitor<T, Allocation_Verifier>;
using Handler_Scope  = Memory_Monitor_Handler_Scope<Allocation_Verifier>;

TEST_CASE("Various mechanisims to initialise binary tree", "[binary_tree]")
{
	using Binary_Tree = dsa::Binary_Tree<int, Allocator_Base>;
	Handler_Scope scope;

	SECTION("Default initialised binary tree has no elements")
	{
		Binary_Tree binary_tree;

		REQUIRE(binary_tree.empty());
		REQUIRE(binary_tree.size() == 0);
	}

	SECTION("Construct using binary_tree initialisation")
	{
		std::initializer_list<int> values{1, 2, 3};
		Binary_Tree binary_tree(values);

		REQUIRE_FALSE(binary_tree.empty());
		REQUIRE(binary_tree.size() == 3);
		REQUIRE_THAT(binary_tree, EqualsRange(values));
	}
}

TEST_CASE("Binary trees can be compared", "[binary_tree]")
{
	using Binary_Tree = dsa::Binary_Tree<int, Allocator_Base>;
	Handler_Scope scope;

	SECTION("Empty Binary trees are equal")
	{
		Binary_Tree lhs;
		Binary_Tree rhs;

		REQUIRE(lhs == rhs);
		REQUIRE_FALSE(lhs != rhs);
	}

	SECTION("Empty Binary tree is unequal to non empty binary tree")
	{
		Binary_Tree lhs;
		Binary_Tree rhs{0};

		REQUIRE_FALSE(lhs == rhs);
		REQUIRE(lhs != rhs);
	}

	SECTION("Binary trees with same element are equal")
	{
		Binary_Tree lhs{0};
		Binary_Tree rhs{0};

		REQUIRE(lhs == rhs);
		REQUIRE_FALSE(lhs != rhs);
	}

	SECTION("Binary trees with subset of elements are unequal")
	{
		Binary_Tree lhs{0, 1};
		Binary_Tree rhs{0, 1, 2};

		REQUIRE_FALSE(lhs == rhs);
		REQUIRE(lhs != rhs);
	}

	SECTION("Binary trees with a differing element are unequal")
	{
		Binary_Tree lhs{0, 1, 2};
		Binary_Tree rhs{0, 1, 3};

		REQUIRE_FALSE(lhs == rhs);
		REQUIRE(lhs != rhs);
	}

	SECTION("Binary trees with the same elements are equal")
	{
		Binary_Tree lhs{0, 1, 2};
		Binary_Tree rhs{0, 1, 2};

		REQUIRE(lhs == rhs);
		REQUIRE_FALSE(lhs != rhs);
	}

	SECTION("Insertion order does not affect equality")
	{
		Binary_Tree lhs{0, 1, 2};
		Binary_Tree rhs{0, 2, 1};

		REQUIRE(lhs == rhs);
		REQUIRE_FALSE(lhs != rhs);
	}
}

TEST_CASE("Binary trees can be copied", "[binary_tree]")
{
	using Binary_Tree = dsa::Binary_Tree<int, Allocator_Base>;
	Handler_Scope scope;

	Binary_Tree binary_tree{0, -1, 1};

	SECTION("Binary trees can be copy constructed from one another")
	{
		Binary_Tree copy(binary_tree);

		REQUIRE(binary_tree == copy);
	}

	SECTION("Binary trees can be copy assigned from one another")
	{
		Binary_Tree copy;
		REQUIRE(binary_tree != copy);

		copy = binary_tree;
		REQUIRE(binary_tree == copy);
	}
}

TEST_CASE("Binary trees can be moved", "[binary_tree]")
{
	using Binary_Tree = dsa::Binary_Tree<int, Allocator_Base>;
	Handler_Scope scope;

	std::initializer_list<int> values{1, 2, 3};
	Binary_Tree temporary(values);

	SECTION("Binary trees can be move constructed from one another")
	{
		Binary_Tree binary_tree(std::move(temporary));

		REQUIRE_THAT(binary_tree, EqualsRange(values));
	}

	SECTION("Binary trees can be move assigned from one another")
	{
		Binary_Tree binary_tree;
		REQUIRE(binary_tree != temporary);

		binary_tree = std::move(temporary);
		REQUIRE_THAT(binary_tree, EqualsRange(values));
	}
}

TEST_CASE("Binary trees can be swapped", "[binary_tree]")
{
	using Binary_Tree = dsa::Binary_Tree<int, Allocator_Base>;
	Handler_Scope scope;

	std::initializer_list<int> values_a{1, 2, 3};
	std::initializer_list<int> values_b{4, 5, 6};

	Binary_Tree binary_tree_a{values_a};
	Binary_Tree binary_tree_b{values_b};

	swap(binary_tree_a, binary_tree_b);

	REQUIRE_THAT(binary_tree_a, EqualsRange(values_b));
	REQUIRE_THAT(binary_tree_b, EqualsRange(values_a));
}

TEST_CASE("Binary trees can be queried for element containment", "[binary_tree]")
{
	using Binary_Tree = dsa::Binary_Tree<int, Allocator_Base>;
	Handler_Scope scope;

	Binary_Tree binary_tree{0, -1, 1};

	SECTION("A present element is marked as contained")
	{
		REQUIRE(binary_tree.contains(1));
	}

	SECTION("An absent element is not marked as contained")
	{
		REQUIRE_FALSE(binary_tree.contains(2));
	}
}

TEST_CASE("Elements can be inserted into the binary tree", "[binary_tree]")
{
	using Binary_Tree = dsa::Binary_Tree<int, Allocator_Base>;
	Handler_Scope scope;

	SECTION("Inserting an element adds it to the binary tree")
	{
		std::initializer_list<int> expected{0};

		Binary_Tree binary_tree;
		binary_tree.insert(0);

		REQUIRE_THAT(binary_tree, EqualsRange(expected));
	}

	Binary_Tree binary_tree{0, -2, 2};

	SECTION("Inserted element is stored in a right node if larger")
	{
		std::initializer_list<int> expected{-2, -1, 0, 2};

		binary_tree.insert(-1);

		REQUIRE_THAT(binary_tree, EqualsRange(expected));
	}

	SECTION("Inserted element is stored in a left node if smaller")
	{
		std::initializer_list<int> expected{-2, 0, 1, 2};

		binary_tree.insert(1);

		REQUIRE_THAT(binary_tree, EqualsRange(expected));
	}
}

TEST_CASE("Elements can be erased from the binary tree", "[binary_tree]")
{
	using Binary_Tree = dsa::Binary_Tree<int, Allocator_Base>;
	Handler_Scope scope;

	SECTION("Erasing last element empties the binary tree")
	{
		Binary_Tree binary_tree{0};

		binary_tree.erase(0);

		REQUIRE(binary_tree.empty());
		REQUIRE(binary_tree == Binary_Tree());
	}

	Binary_Tree binary_tree{
	            0,
	     -3,           3,
	-4,      -1,   1,      4,
	      -2,         2};

	SECTION("Erasing left leaf preserves other nodes")
	{
		Binary_Tree expected{
			    0,
		     -3,           3,
		         -1,   1,      4,
		      -2,         2};
		binary_tree.erase(-4);

		REQUIRE(Binary_Tree::same_structure(binary_tree, expected));
	}

	SECTION("Erasing right leaf preserves other nodes")
	{
		Binary_Tree expected{
			    0,
		     -3,           3,
		-4,      -1,   1,
		      -2,         2};
		binary_tree.erase(4);

		REQUIRE(Binary_Tree::same_structure(binary_tree, expected));
	}

	SECTION("Erasing parent with single left child promotes it")
	{
		Binary_Tree expected{
			    0,
		     -3,           3,
		-4,      -2,   1,      4,
				  2};
		binary_tree.erase(-1);

		REQUIRE(Binary_Tree::same_structure(binary_tree, expected));
	}

	SECTION("Erasing parent with single right child promotes it")
	{
		Binary_Tree expected{
			    0,
		     -3,           3,
		-4,      -1,   2,      4,
		      -2};
		binary_tree.erase(1);

		REQUIRE(Binary_Tree::same_structure(binary_tree, expected));
	}

	SECTION(
	    "Erasing a parent with both children promotes the right most child "
	    "of the left node")
	{
		Binary_Tree expected{
			    0,
		     -3,           2,
		-4,      -1,   1,      4,
		      -2};
		binary_tree.erase(3);

		REQUIRE(Binary_Tree::same_structure(binary_tree, expected));
	}

	SECTION(
	    "Erasing a parent with both children promotes the smaller element")
	{
		Binary_Tree expected{
		            -1,
		     -3,           3,
		-4,      -2,   1,      4,
		                  2};
		binary_tree.erase(0);

		REQUIRE(Binary_Tree::same_structure(binary_tree, expected));
	}

	SECTION("Cler erases all elements from the tree")
	{
		binary_tree.clear();

		REQUIRE(binary_tree.empty());
		REQUIRE(binary_tree == Binary_Tree());
	}
}

} // namespace test
