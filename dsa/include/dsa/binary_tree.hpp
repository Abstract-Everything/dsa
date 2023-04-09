#ifndef DSA_BINARY_TREE_HPP
#define DSA_BINARY_TREE_HPP

#include <dsa/allocator_traits.hpp>
#include <dsa/default_allocator.hpp>

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
	using Alloc_Traits = Allocator_Traits<Allocator_Base<Binary_Tree_Node>>;
	using Allocator    = typename Alloc_Traits::Allocator;
	using Pointer      = typename Alloc_Traits::Pointer;
	using Const_Pointer = typename Alloc_Traits::Const_Pointer;

	using Satellite_Alloc_Traits    = Allocator_Traits<Allocator_Base<Satellite_t>>;
	using Satellite                 = typename Satellite_Alloc_Traits::Value;
	using Satellite_Pointer         = typename Satellite_Alloc_Traits::Reference;
	using Satellite_Const_Pointer   = typename Satellite_Alloc_Traits::Const_Reference;
	using Satellite_Reference       = typename Satellite_Alloc_Traits::Reference;
	using Satellite_Const_Reference = typename Satellite_Alloc_Traits::Const_Reference;

	template<bool Is_Const>
	class Iterator_Detail
	{
	 private:
		using Node_Pointer = std::conditional_t<
		    Is_Const,
		    typename Binary_Tree_Node::Const_Pointer,
		    typename Binary_Tree_Node::Pointer>;

		using Pointer = std::conditional_t<
		    Is_Const,
		    typename Binary_Tree_Node::Satellite_Const_Pointer,
		    typename Binary_Tree_Node::Satellite_Pointer>;

		using Reference = std::conditional_t<
		    Is_Const,
		    typename Binary_Tree_Node::Satellite_Const_Reference,
		    typename Binary_Tree_Node::Satellite_Reference>;

	 public:
		using iterator_category = std::forward_iterator_tag;
		using difference_type   = std::ptrdiff_t;
		using value_type        = Satellite;
		using reference         = Reference;
		using pointer           = Pointer;

		explicit Iterator_Detail(Node_Pointer node) : m_node(node)
		{
		}

		Iterator_Detail operator++()
		{
			m_node = next(m_node);
			Iterator_Detail iterator(m_node);
			return iterator;
		}

		bool operator==(Iterator_Detail const &iterator) const
		{
			return m_node == iterator.m_node;
		}

		bool operator!=(Iterator_Detail const &iterator) const
		{
			return !this->operator==(iterator);
		}

		Reference operator*() const
		{
			return m_node->m_satellite;
		}

		Pointer operator->() const
		{
			return m_node->m_satellite;
		}

	 private:
		Node_Pointer m_node;

		Node_Pointer next(Node_Pointer node)
		{
			if (node == nullptr)
			{
				return nullptr;
			}

			if (node->m_right != nullptr)
			{
				node = node->m_right;
				while (node->m_left != nullptr)
				{
					node = node->m_left;
				}
				return node;
			}

			if (node->m_parent == nullptr)
			{
				return nullptr;
			}

			if (node->m_parent->m_left == node)
			{
				return node->m_parent;
			}

			while (node->m_parent != nullptr
			       && node->m_parent->m_right == node)
			{
				node = node->m_parent;
			}
			return node->m_parent;
		}
	};

 public:
	using Iterator       = Iterator_Detail<false>;
	using Const_Iterator = Iterator_Detail<true>;

	Pointer   m_parent;
	Pointer   m_left;
	Pointer   m_right;
	Satellite m_satellite;

	template<typename... Arguments>
	explicit Binary_Tree_Node(Arguments &&...arguments)
	    : m_parent(nullptr)
	    , m_left(nullptr)
	    , m_right(nullptr)
	    , m_satellite(std::forward<Arguments>(arguments)...)
	{
	}

	friend auto operator<<(std::ostream &stream, Binary_Tree_Node const &node)
	    -> std::ostream &
	{
		// clang-format off
		return stream << '{'
				<< node.m_left
				<< ',' << node.m_parent
				<< ',' << node.m_right
				<< ',' << node.m_satellite
			<< '}';
		// clang-format on
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
	using Node_Traits = Allocator_Traits<Allocator_Base<Node>>;
	using Node_Allocator     = typename Node_Traits::Allocator;
	using Node_Pointer       = typename Node_Traits::Pointer;
	using Node_Const_Pointer = typename Node_Traits::Const_Pointer;

	using Alloc_Traits = Allocator_Traits<Allocator_Base<Value_t>>;

 public:
	using Allocator       = typename Alloc_Traits::Allocator;
	using Value           = typename Alloc_Traits::Value;
	using Reference       = typename Alloc_Traits::Reference;
	using Const_Reference = typename Alloc_Traits::Const_Reference;
	using Pointer         = typename Alloc_Traits::Pointer;
	using Iterator        = typename Node::Iterator;
	using Const_Iterator  = typename Node::Const_Iterator;

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

	[[nodiscard]] Iterator begin()
	{
		Node_Pointer node = m_head;
		while (node != nullptr && node->m_left != nullptr)
		{
			node = node->m_left;
		}
		return Iterator(node);
	}

	[[nodiscard]] Const_Iterator begin() const
	{
		Node_Pointer node = m_head;
		while (node != nullptr && node->m_left != nullptr)
		{
			node = node->m_left;
		}
		return Const_Iterator(node);
	}

	[[nodiscard]] Iterator end()
	{
		return Iterator(nullptr);
	}

	[[nodiscard]] Const_Iterator end() const
	{
		return Const_Iterator(nullptr);
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
	[[nodiscard]] bool contains(const Value_t &value) const
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
	void insert(Value_t value)
	{
		Node_Pointer insert = create_node(std::move(value));

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

		*node_ptr        = insert;
		insert->m_parent = parent;
	}

	/**
	 * @brief Removes the given element from the binary tree. The behaviour
	 * is undefined if the value is not present in the binary tree
	 */
	void erase(Value_t value)
	{
		Node_Pointer *pointer = &m_head;
		while ((*pointer)->m_satellite != value)
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
			(*pointer)->m_parent = node->m_parent;

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

		Node_Pointer root = create_node(subtree->m_satellite);
		root->m_parent = parent;
		root->m_left   = copy_subtree(root, subtree->m_left);
		root->m_right  = copy_subtree(root, subtree->m_right);
		return root;
	}

	template<typename... Arguments>
	Node_Pointer create_node(Arguments &&...arguments)
	{
		Node_Pointer pointer = Node_Traits::allocate(m_allocator, 1);
		Node_Traits::construct(
		    m_allocator,
		    pointer,
		    std::forward<Arguments>(arguments)...);
		return pointer;
	}

	void destroy_node(Node_Pointer node)
	{
		Node_Traits::destroy(m_allocator, node);
		Node_Traits::deallocate(m_allocator, node, 1);
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
