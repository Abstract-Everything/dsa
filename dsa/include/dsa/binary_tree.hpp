#ifndef DSA_BINARY_TREE_HPP
#define DSA_BINARY_TREE_HPP

#include "dsa/weak_pointer.hpp"

#include <memory>
#include <cassert>

namespace dsa
{

// ToDo: Use concepts to check that Value is sortable
template<
    typename Value_t,
    template<typename> typename Pointer_Base   = Weak_Pointer,
    template<typename> typename Allocator_Base = std::allocator>
class Binary_Tree
{
	class Node;
	using Allocator = Allocator_Base<Node>;
	using Pointer   = Pointer_Base<Node>;

 public:
	using Value = Value_t;

	explicit Binary_Tree(const Allocator &allocator = Allocator{})
	    : m_allocator(allocator)
	{
	}

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

	[[nodiscard]] bool empty() const
	{
		return m_head == nullptr;
	}

	[[nodiscard]] std::size_t size() const
	{
		return count_nodes(m_head);
	}

	void clear()
	{
		delete_node(m_head);
		m_head = nullptr;
	}

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

	void insert(Value value)
	{
		Pointer insert = m_allocator.allocate(1);
		insert->value  = value;
		insert->left   = nullptr;
		insert->right  = nullptr;

		Pointer *node_ptr = &m_head;
		while (*node_ptr != nullptr)
		{
			Pointer &node = *node_ptr;
			node_ptr = insert->value < node->value ? &node->left
							       : &node->right;
		}

		*node_ptr = insert;
	}

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
		return subtree_equal(lhs.m_head, rhs.m_head);
	}

	friend bool operator!=(Binary_Tree const &lhs, Binary_Tree const &rhs) noexcept
	{
		return !(lhs == rhs);
	}

 private:
	class Node
	{
	 public:
		Value   value;
		Pointer left;
		Pointer right;
	};

	Allocator m_allocator;

	Pointer m_head;

	std::size_t count_nodes(Pointer node) const
	{
		if (node == nullptr)
		{
			return 0;
		}
		return 1 + count_nodes(node->left) + count_nodes(node->right);
	}

	[[nodiscard]] static bool subtree_equal(Pointer lhs, Pointer rhs)
	{
		return (lhs == rhs)
		       || (lhs != nullptr && rhs != nullptr
			   && lhs->value == rhs->value
			   && subtree_equal(lhs->left, rhs->left)
			   && subtree_equal(lhs->right, rhs->right));
	}

	[[nodiscard]] Pointer copy_subtree(Pointer subtree)
	{
		if (subtree == nullptr)
		{
			return nullptr;
		}

		Pointer root = m_allocator.allocate(1);
		root->value  = subtree->value;
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
