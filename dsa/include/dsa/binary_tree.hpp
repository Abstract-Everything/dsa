#ifndef DSA_BINARY_TREE_HPP
#define DSA_BINARY_TREE_HPP

#include <dsa/allocator_traits.hpp>
#include <dsa/default_allocator.hpp>
#include <dsa/node.hpp>

#include <cassert>
#include <memory>

namespace dsa
{

namespace detail
{

template<typename Satellite_t, template<typename> typename Allocator_Base>
class Binary_Tree_Node
{
 private:
	friend class Node_Traits<Binary_Tree_Node>;

	using Alloc_Traits = Allocator_Traits<Allocator_Base<Binary_Tree_Node>>;
	using Allocator    = typename Alloc_Traits::Allocator;
	using Pointer      = typename Alloc_Traits::Pointer;
	using Const_Pointer = typename Alloc_Traits::Const_Pointer;

	using Satellite_Alloc_Traits =
	    Allocator_Traits<Allocator_Base<Satellite_t>>;
	using Satellite = typename Satellite_Alloc_Traits::Value;

 public:
	Satellite m_satellite;
	Pointer   m_parent;
	Pointer   m_left;
	Pointer   m_right;

	void initialise()
	{
		m_parent = nullptr;
		m_left   = nullptr;
		m_right  = nullptr;
	}
};

} // namespace detail

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
template<typename Value_t, template<typename> typename Allocator_Base = Default_Allocator>
class Binary_Tree
{
 private:
	using Node        = detail::Binary_Tree_Node<Value_t, Allocator_Base>;
	using Node_Traits = detail::Node_Traits<Node>;
	using Node_Allocator     = typename Node_Traits::Allocator;
	using Node_Pointer       = typename Node_Traits::Pointer;
	using Node_Const_Pointer = typename Node_Traits::Const_Pointer;

	using Alloc_Traits = Allocator_Traits<Allocator_Base<Value_t>>;

 public:
	using Allocator = typename Alloc_Traits::Allocator;
	using Value     = typename Alloc_Traits::Value;
	using Pointer   = typename Alloc_Traits::Pointer;

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
	    , m_head(copy_subtree(nullptr, binary_tree.m_head))
	{
	}

	Binary_Tree(Binary_Tree &&binary_tree) noexcept
	    : m_allocator(binary_tree.m_allocator)
	    , m_head(binary_tree.m_head)
	{
		binary_tree.m_head = nullptr;
	}

	friend void swap(Binary_Tree &lhs, Binary_Tree &rhs)
	{
		using std::swap;
		swap(lhs.m_allocator, rhs.m_allocator);
		swap(lhs.m_head, rhs.m_head);
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
		Node_Const_Pointer node = m_head;
		while (node != nullptr)
		{
			if (node->m_satellite == value)
			{
				return true;
			}
			node = value < node->m_satellite ? node->m_left
							 : node->m_right;
		}
		return false;
	}

	/**
	 * @brief Adds the given element into the binary tree
	 */
	void insert(Value value)
	{
		Node_Pointer insert =
		    Node_Traits::create_node(m_allocator, std::move(value));

		Node_Pointer  parent   = nullptr;
		Node_Pointer *node_ptr = &m_head;
		while (*node_ptr != nullptr)
		{
			parent             = *node_ptr;
			Node_Pointer &node = *node_ptr;
			node_ptr = insert->m_satellite < node->m_satellite
				       ? &node->m_left
				       : &node->m_right;
		}

		*node_ptr       = insert;
		insert->m_parent = parent;
	}

	/**
	 * @brief Removes the given element from the binary tree. The behaviour
	 * is undefined if the value is not present in the binary tree
	 */
	void erase(Value value)
	{
		Node_Pointer *pointer = &m_head;
		while (*pointer != nullptr && (*pointer)->m_satellite != value)
		{
			Node_Pointer node = *pointer;
			pointer = value < node->m_satellite ? &node->m_left
							    : &node->m_right;
		}

		Node_Pointer node = *pointer;
		if (node->m_left != nullptr)
		{
			using std::swap;
			Node_Pointer previous = extract_previous_node(node);
			swap(node->m_satellite, previous->m_satellite);
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
	Node_Allocator m_allocator;

	Node_Pointer m_head = nullptr;

	std::size_t count_nodes(Node_Const_Pointer node) const
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
		       || (binary_tree.contains(node->m_satellite)
			   && is_subset(node->m_left, binary_tree)
			   && is_subset(node->m_right, binary_tree));
	}

	[[nodiscard]] static bool compare_structure(Node_Pointer lhs, Node_Pointer rhs)
	{
		return (lhs == rhs)
		       || (lhs != nullptr && rhs != nullptr
			   && lhs->m_satellite == rhs->m_satellite
			   && compare_structure(lhs->m_left, rhs->m_left)
			   && compare_structure(lhs->m_right, rhs->m_right));
	}

	[[nodiscard]] Node_Pointer copy_subtree(Node_Pointer parent, Node_Pointer subtree)
	{
		if (subtree == nullptr)
		{
			return nullptr;
		}

		Node_Pointer root =
		    Node_Traits::create_node(m_allocator, subtree->m_satellite);
		root->m_parent = parent;
		root->m_left  = copy_subtree(root, subtree->m_left);
		root->m_right = copy_subtree(root, subtree->m_right);
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

		Node_Traits::destroy_node(m_allocator, node);
	}

	Node_Pointer extract_previous_node(Node_Pointer node)
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
