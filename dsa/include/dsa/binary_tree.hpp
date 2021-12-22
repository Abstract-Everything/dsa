#ifndef DSA_BINARY_TREE_HPP
#define DSA_BINARY_TREE_HPP

#include "dsa/weak_pointer.hpp"

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

	~Binary_Tree()
	{
		clear();
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
			Pointer remove = node->right;
			*node          = std::move(*node->right);

			remove->left  = nullptr;
			remove->right = nullptr;
			delete_node(remove);
		}
		else
		{
			*pointer = nullptr;
			delete_node(node);
		}
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
