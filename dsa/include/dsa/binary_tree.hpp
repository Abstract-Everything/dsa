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
	using Node_Pointer   = Pointer_Base<Node>;

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
		Node_Pointer node = m_head;
		while (node != nullptr)
		{
			if (node->m_value == value)
			{
				return true;
			}
			node = value < node->m_value ? node->m_left
						     : node->m_right;
		}
		return false;
	}

	/**
	 * @brief Adds the given element into the binary tree
	 */
	void insert(Value value)
	{
		Node_Pointer insert = construct_node(std::move(value));

		Node_Pointer *node_ptr = &m_head;
		while (*node_ptr != nullptr)
		{
			Node_Pointer &node = *node_ptr;
			node_ptr           = insert->m_value < node->m_value
						 ? &node->m_left
						 : &node->m_right;
		}

		*node_ptr = insert;
	}

	/**
	 * @brief Removes the given element from the binary tree. The behaviour
	 * is undefined if the value is not present in the binary tree
	 */
	void erase(Value value)
	{
		Node_Pointer *pointer = &m_head;
		while (*pointer != nullptr && (*pointer)->m_value != value)
		{
			Node_Pointer node = *pointer;
			pointer = value < node->m_value ? &node->m_left
							: &node->m_right;
		}

		Node_Pointer node = *pointer;
		if (node->m_left != nullptr)
		{
			using std::swap;
			Node_Pointer previous = extract_previous_node(node);
			swap(node->m_value, previous->m_value);
			delete_node(previous);
		}
		else if (node->m_right != nullptr)
		{
			*pointer = node->m_right;

			node->m_right = nullptr;
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
		Value        m_value;
		Node_Pointer m_left;
		Node_Pointer m_right;
	};

	Node_Allocator m_allocator;

	Node_Pointer m_head;

	std::size_t count_nodes(Node_Pointer node) const
	{
		if (node == nullptr)
		{
			return 0;
		}
		return 1 + count_nodes(node->m_left) + count_nodes(node->m_right);
	}

	[[nodiscard]] static bool is_subset(
	    Node_Pointer       node,
	    Binary_Tree const &binary_tree)
	{
		return (node == nullptr)
		       || (binary_tree.contains(node->m_value)
			   && is_subset(node->m_left, binary_tree)
			   && is_subset(node->m_right, binary_tree));
	}

	[[nodiscard]] static bool compare_structure(Node_Pointer lhs, Node_Pointer rhs)
	{
		return (lhs == rhs)
		       || (lhs != nullptr && rhs != nullptr
			   && lhs->m_value == rhs->m_value
			   && compare_structure(lhs->m_left, rhs->m_left)
			   && compare_structure(lhs->m_right, rhs->m_right));
	}

	[[nodiscard]] Node_Pointer copy_subtree(Node_Pointer subtree)
	{
		if (subtree == nullptr)
		{
			return nullptr;
		}

		Node_Pointer root = construct_node(subtree->m_value);
		root->m_left      = copy_subtree(subtree->m_left);
		root->m_right     = copy_subtree(subtree->m_right);
		return root;
	}

	void delete_node(Node_Pointer node)
	{
		if (node == nullptr)
		{
			return;
		}

		delete_node(node->m_left);
		delete_node(node->m_right);

		destroy_node(node);
	}

	[[nodiscard]] Node_Pointer construct_node(Value value)
	{
		Node_Pointer node = m_allocator.allocate(1);
		node->m_left      = nullptr;
		node->m_right     = nullptr;

		std::allocator<Value> allocator;
		std::allocator_traits<std::allocator<Value>>::construct(
		    allocator,
		    &(node->m_value),
		    std::move(value));

		return node;
	}

	void destroy_node(Node_Pointer node)
	{
		std::allocator<Value> allocator;
		std::allocator_traits<std::allocator<Value>>::destroy(
		    allocator,
		    &node->m_value);

		m_allocator.deallocate(node.get(), 1);
	}

	Node_Pointer extract_previous_node(Node_Pointer node) const
	{
		assert(node->m_left != nullptr && "");
		if (node->m_left->m_right == nullptr)
		{
			Node_Pointer result = node->m_left;
			node->m_left        = result->m_left;
			result->m_left      = nullptr;
			return result;
		}

		Node_Pointer parent = node->m_left;
		while (parent->m_right->m_right != nullptr)
		{
			parent = parent->m_right;
		}

		Node_Pointer result = parent->m_right;
		parent->m_right     = result->m_left;
		result->m_left      = nullptr;
		return result;
	}
};

} // namespace dsa

#endif
