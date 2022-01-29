#ifndef DSA_BINARY_TREE_HPP
#define DSA_BINARY_TREE_HPP

#include "dsa/weak_pointer.hpp"

#include <cassert>
#include <memory>

namespace dsa
{

// ToDo: Use concepts to check that Value is sortable

/**
 * @brief Holds a set of sortable elements in a binary tree.
 *
 * @ingroup containers
 *
 * @tparam Value_t: The type of element to store
 * @tparam Pointer_Base: The type of pointer used to refer to memory
 * @tparam Allocator_Base: The type of allocator used for memory management
 *
 */
template<
    typename Value_t,
    template<typename> typename Pointer_Base   = Weak_Pointer,
    template<typename> typename Allocator_Base = std::allocator>
class Binary_Tree
{
	class Node;
	using Node_Allocator = Allocator_Base<Node>;
	using Pointer        = Pointer_Base<Node>;

 public:
	using Allocator = Allocator_Base<Value_t>;
	using Value     = Value_t;

	/**
	 * @brief Constructs an empty binary tree
	 */
	explicit Binary_Tree(const Allocator &allocator = Allocator{})
	    : m_allocator(allocator)
	{
	}

	/**
	 * @brief Constructs a binary tree filled with the given values
	 */
	Binary_Tree(
	    std::initializer_list<Value_t> values,
	    const Allocator               &allocator = Allocator())
	    : m_allocator(allocator)
	{
		for (auto value : values)
		{
			insert(value);
		}
	}

	~Binary_Tree()
	{
		clear();
	}

	Binary_Tree(Binary_Tree const &binary_tree)
	    : m_allocator(binary_tree.m_allocator)
	    , m_head(copy_subtree(binary_tree.m_head))
	{
	}

	Binary_Tree(Binary_Tree &&binary_tree) noexcept
	    : m_allocator(binary_tree.m_allocator)
	    , m_head(binary_tree.m_head)
	{
		binary_tree.m_head = nullptr;
	}

	void swap(Binary_Tree &lhs, Binary_Tree &rhs)
	{
		std::swap(lhs.m_allocator, rhs.m_allocator);
		std::swap(lhs.m_head, rhs.m_head);
	}

	Binary_Tree &operator=(Binary_Tree binary_tree) noexcept
	{
		swap(*this, binary_tree);
		return *this;
	}

	/**
	 * @brief Returns true if the binary tree contains no elements
	 */
	[[nodiscard]] bool empty() const
	{
		return m_head == nullptr;
	}

	/**
	 * @brief Gets the number of elements currently in the binary tree.
	 * Note: This operation is not constant as the size is not cached
	 */
	[[nodiscard]] std::size_t size() const
	{
		return count_nodes(m_head);
	}

	/**
	 * @brief Clears all elements from the binary tree
	 */
	void clear()
	{
		delete_node(m_head);
		m_head = nullptr;
	}

	/**
	 * @brief Returns true if the binary tree contains the given element
	 */
	[[nodiscard]] bool contains(const Value &value) const
	{
		Pointer node = m_head;
		while (node != nullptr)
		{
			if (node->value == value)
			{
				return true;
			}
			node = value < node->value ? node->left : node->right;
		}
		return false;
	}

	/**
	 * @brief Adds the given element into the binary tree
	 */
	void insert(Value value)
	{
		Pointer insert = construct_node(std::move(value));

		Pointer *node_ptr = &m_head;
		while (*node_ptr != nullptr)
		{
			Pointer &node = *node_ptr;
			node_ptr = insert->value < node->value ? &node->left
							       : &node->right;
		}

		*node_ptr = insert;
	}

	/**
	 * @brief Removes the given element from the binary tree. The behaviour
	 * is undefined if the value is not present in the binary tree
	 */
	void erase(Value value)
	{
		Pointer *pointer = &m_head;
		while (*pointer != nullptr && (*pointer)->value != value)
		{
			Pointer node = *pointer;
			pointer =
			    value < node->value ? &node->left : &node->right;
		}

		Pointer node = *pointer;
		if (node->left != nullptr)
		{
			using std::swap;
			Pointer previous = extract_previous_node(node);
			swap(node->value, previous->value);
			delete_node(previous);
		}
		else if (node->right != nullptr)
		{
			*pointer = node->right;

			node->right = nullptr;
			delete_node(node);
		}
		else
		{
			*pointer = nullptr;
			delete_node(node);
		}
	}

	friend bool operator==(Binary_Tree const &lhs, Binary_Tree const &rhs) noexcept
	{
		return lhs.size() == rhs.size() && is_subset(lhs.m_head, rhs);
	}

	friend bool operator!=(Binary_Tree const &lhs, Binary_Tree const &rhs) noexcept
	{
		return !(lhs == rhs);
	}

	/**
	 * @brief Returns true if the node structure and every nodes value is
	 * the same for both trees
	 */
	[[nodiscard]] static bool same_structure(
	    Binary_Tree const &lhs,
	    Binary_Tree const &rhs) noexcept
	{
		return compare_structure(lhs.m_head, rhs.m_head);
	}

 private:
	class Node
	{
	 public:
		Value   value;
		Pointer left;
		Pointer right;
	};

	Node_Allocator m_allocator;

	Pointer m_head;

	std::size_t count_nodes(Pointer node) const
	{
		if (node == nullptr)
		{
			return 0;
		}
		return 1 + count_nodes(node->left) + count_nodes(node->right);
	}

	[[nodiscard]] static bool is_subset(Pointer node, Binary_Tree const &binary_tree)
	{
		return (node == nullptr)
		       || (binary_tree.contains(node->value)
			   && is_subset(node->left, binary_tree)
			   && is_subset(node->right, binary_tree));
	}

	[[nodiscard]] static bool compare_structure(Pointer lhs, Pointer rhs)
	{
		return (lhs == rhs)
		       || (lhs != nullptr && rhs != nullptr
			   && lhs->value == rhs->value
			   && compare_structure(lhs->left, rhs->left)
			   && compare_structure(lhs->right, rhs->right));
	}

	[[nodiscard]] Pointer copy_subtree(Pointer subtree)
	{
		if (subtree == nullptr)
		{
			return nullptr;
		}

		Pointer root = construct_node(subtree->value);
		root->left   = copy_subtree(subtree->left);
		root->right  = copy_subtree(subtree->right);
		return root;
	}

	void delete_node(Pointer node)
	{
		if (node == nullptr)
		{
			return;
		}

		delete_node(node->left);
		delete_node(node->right);

		destroy_node(node);
	}

	[[nodiscard]] Pointer construct_node(Value value)
	{
		Pointer node = m_allocator.allocate(1);
		node->left   = nullptr;
		node->right  = nullptr;

		std::allocator<Value> allocator;
		std::allocator_traits<std::allocator<Value>>::construct(
		    allocator,
		    &(node->value),
		    std::move(value));

		return node;
	}

	void destroy_node(Pointer node)
	{
		std::allocator<Value> allocator;
		std::allocator_traits<std::allocator<Value>>::destroy(
		    allocator,
		    &node->value);

		m_allocator.deallocate(node.get(), 1);
	}

	Pointer extract_previous_node(Pointer node) const
	{
		assert(node->left != nullptr && "");
		if (node->left->right == nullptr)
		{
			Pointer result = node->left;
			node->left     = result->left;
			result->left   = nullptr;
			return result;
		}

		Pointer parent = node->left;
		while (parent->right->right != nullptr)
		{
			parent = parent->right;
		}

		Pointer result = parent->right;
		parent->right  = result->left;
		result->left   = nullptr;
		return result;
	}
};

} // namespace dsa

#endif
