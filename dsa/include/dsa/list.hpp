#ifndef DSA_LIST_HPP
#define DSA_LIST_HPP

#include <dsa/allocator_traits.hpp>
#include <dsa/default_allocator.hpp>

#include <cstddef>
#include <memory>

namespace dsa
{

namespace list::detail
{

template<typename Value_t, template<typename> typename Allocator_Base>
class Node
{
 private:
	using Value_Alloc_Traits = Allocator_Traits<Allocator_Base<Value_t>>;
	using Value              = typename Value_Alloc_Traits::Value;
	using Value_Allocator    = typename Value_Alloc_Traits::Allocator;

	using Alloc_Traits = Allocator_Traits<Allocator_Base<Node>>;

 public:
	using Allocator     = typename Alloc_Traits::Allocator;
	using Pointer       = typename Alloc_Traits::Pointer;
	using Const_Pointer = typename Alloc_Traits::Const_Pointer;

	/**
	 * @brief Allocates a node and constructs the held value.
	 */
	[[nodiscard]] static Pointer create_node(Allocator allocator, Value value)
	{
		Pointer pointer = Alloc_Traits::allocate(allocator, 1);
		pointer->m_next = nullptr;

		Value_Allocator value_allocator(allocator);
		Value_Alloc_Traits::construct(
		    value_allocator,
		    &pointer->m_value,
		    std::move(value));

		return pointer;
	}

	/**
	 * @brief Destroys the value of the node and deallocates its
	 * storage. This function does not recursively destroy nodes.
	 */
	static void destroy_node(Allocator allocator, Pointer node)
	{
		Value_Allocator value_allocator(allocator);
		Value_Alloc_Traits::destroy(value_allocator, &node->m_value);
		Alloc_Traits::deallocate(allocator, node, 1);
	}

 public:
	Node() = default;

	Value   m_value = Value();
	Pointer m_next  = nullptr;
};

} // namespace list::detail

// ToDo: Use iterators to make insertion/ deletion constant time

/**
 * @brief Holds a set of non contigious elements in a singly linked list.
 *
 * @ingroup containers
 *
 * @tparam Value_t: The type of element to store
 * @tparam Pointer_Base: The type of pointer used to refer to memory
 * @tparam Allocator_Base: The type of allocator used for memory management
 *
 */
template<typename Value_t, template<typename> typename Allocator_Base = Default_Allocator>
class List
{
 private:
	using Alloc_Traits       = Allocator_Traits<Allocator_Base<Value_t>>;
	using Node               = list::detail::Node<Value_t, Allocator_Base>;
	using Node_Allocator     = typename Node::Allocator;
	using Node_Pointer       = typename Node::Pointer;
	using Node_Const_Pointer = typename Node::Const_Pointer;

 public:
	using Allocator = typename Alloc_Traits::Allocator;
	using Value     = typename Alloc_Traits::Value;
	using Pointer   = typename Alloc_Traits::Pointer;

 public:
	/**
	 * @brief Constructs an empty list
	 */
	explicit List(const Allocator &allocator = Allocator{})
	    : m_allocator(allocator)
	{
	}

	/**
	 * @brief Constructs a list filled with the given values
	 */
	List(
	    std::initializer_list<Value_t> values,
	    const Allocator               &allocator = Allocator())
	    : m_allocator(allocator)
	{
		Node_Pointer *next = &m_head;
		for (auto value : values)
		{
			Node_Pointer &to = *next;
			to = Node::create_node(m_allocator, value);

			next = &to->m_next;
		}
	}

	~List()
	{
		clear();
	}

	List(List const &list) : m_allocator(list.m_allocator)
	{
		Node_Pointer  from = list.m_head;
		Node_Pointer *next = &m_head;
		while (from != nullptr)
		{
			Node_Pointer &to = *next;
			to = Node::create_node(m_allocator, from->m_value);

			next = &to->m_next;
			from = from->m_next;
		}
	}

	friend void swap(List &lhs, List &rhs)
	{
		using std::swap;
		swap(lhs.m_allocator, rhs.m_allocator);
		swap(lhs.m_head, rhs.m_head);
	}

	List(List &&list) noexcept : m_allocator(list.m_allocator)
	{
		swap(*this, list);
	}

	List &operator=(List list) noexcept
	{
		swap(*this, list);
		return *this;
	}

	[[nodiscard]] Value &operator[](std::size_t index)
	{
		return at(index)->m_value;
	}

	[[nodiscard]] const Value &operator[](std::size_t index) const
	{
		return at(index)->m_value;
	}

	/**
	 * @brief Gets the number of elements currently in the list.
	 * Note: This operation is not constant as the size is not cached
	 */
	[[nodiscard]] std::size_t size() const
	{
		std::size_t  list_size = 0;
		Node_Pointer node      = m_head;
		while (node != nullptr)
		{
			node = node->m_next;
			list_size++;
		}
		return list_size;
	}

	/**
	 * @brief Gets the first element in the list. This is undefined
	 * behaviour if the list is empty
	 */
	[[nodiscard]] Value &front()
	{
		return m_head->m_value;
	}

	/**
	 * @brief Gets the first element in the list. This is undefined
	 * behaviour if the list is empty
	 */
	[[nodiscard]] const Value &front() const
	{
		return m_head->m_value;
	}

	/**
	 * @brief Returns true if the list contains no elements
	 */
	[[nodiscard]] bool empty() const
	{
		return m_head == nullptr;
	}

	/**
	 * @brief Clears all elements from the list
	 */
	void clear()
	{
		Node_Pointer node = m_head;
		while (node != nullptr)
		{
			Node_Pointer next = node->m_next;
			Node::destroy_node(m_allocator, node);
			node = next;
		}
		m_head = nullptr;
	}

	/**
	 * @brief Inserts the given value at the front of the list
	 */
	void prepend(Value value)
	{
		insert(0, std::move(value));
	}

	/**
	 * @brief Inserts the given value at the given index. The behaviour is
	 * undefined if the index is outside of the range: [0, size()]
	 */
	void insert(std::size_t index, Value value)
	{
		Node_Pointer node =
		    Node::create_node(m_allocator, std::move(value));

		if (index == 0)
		{
			node->m_next = m_head;
			m_head       = node;
			return;
		}

		Node_Pointer previous = at(index - 1);
		node->m_next          = previous->m_next;
		previous->m_next      = node;
	}

	/**
	 * @brief Erases the value at the front of the list. The behaviour is
	 * undefined if the list is empty
	 */
	void detatch_front()
	{
		erase(0);
	}

	/**
	 * @brief Erases the value at the given index. The behaviour is
	 * undefined if the index is outside of the list size.
	 */
	void erase(std::size_t index)
	{
		if (index == 0)
		{
			Node_Pointer remove = m_head;
			m_head              = m_head->m_next;
			Node::destroy_node(m_allocator, remove);
			return;
		}

		Node_Pointer previous = at(index - 1);
		Node_Pointer remove   = previous->m_next;
		previous->m_next      = remove->m_next;
		Node::destroy_node(m_allocator, remove);
	}

	friend bool operator==(List const &lhs, List const &rhs) noexcept
	{
		Node_Pointer lhs_node = lhs.m_head;
		Node_Pointer rhs_node = rhs.m_head;
		while (lhs_node != nullptr && rhs_node != nullptr)
		{
			if (lhs_node->m_value != rhs_node->m_value)
			{
				return false;
			}

			lhs_node = lhs_node->m_next;
			rhs_node = rhs_node->m_next;
		}

		return lhs_node == nullptr && rhs_node == nullptr;
	}

	friend bool operator!=(List const &lhs, List const &rhs) noexcept
	{
		return !(lhs == rhs);
	}

 private:
	Node_Allocator m_allocator;

	Node_Pointer m_head = nullptr;

	/**
	 * @brief Gets the node at the given index. The behaviour is undefined
	 * if the index is outside of the list range
	 */
	Node_Pointer at(std::size_t index)
	{
		Node_Pointer node = m_head;
		for (std::size_t i = 0; i < index; ++i)
		{
			node = node->m_next;
		}
		return node;
	}

	/**
	 * @brief Gets the node at the given index. The behaviour is undefined
	 * if the index is outside of the list range
	 */
	Node_Const_Pointer at(std::size_t index) const
	{
		Node_Const_Pointer node = m_head;
		for (std::size_t i = 0; i < index; ++i)
		{
			node = node->m_next;
		}
		return node;
	}
};

} // namespace dsa

#endif
