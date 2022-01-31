#ifndef DSA_LIST_HPP
#define DSA_LIST_HPP

#include <dsa/allocator_traits.hpp>
#include <dsa/default_allocator.hpp>
#include <dsa/node.hpp>

#include <cstddef>
#include <memory>

namespace dsa
{

namespace detail
{

template<typename Satellite_t, template<typename> typename Allocator_Base>
class List_Node
{
 private:
	friend class Node_Traits<List_Node>;

	using Alloc_Traits    = Allocator_Traits<Allocator_Base<List_Node>>;
	using Allocator       = typename Alloc_Traits::Allocator;
	using Pointer         = typename Alloc_Traits::Pointer;
	using Const_Pointer   = typename Alloc_Traits::Const_Pointer;

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
		    typename List_Node::Const_Pointer,
		    typename List_Node::Pointer>;

		using Pointer = std::conditional_t<
		    Is_Const,
		    typename List_Node::Satellite_Const_Pointer,
		    typename List_Node::Satellite_Pointer>;

		using Reference = std::conditional_t<
		    Is_Const,
		    typename List_Node::Satellite_Const_Reference,
		    typename List_Node::Satellite_Reference>;

	 public:
		explicit Iterator_Detail(Node_Pointer node) : m_node(node)
		{
		}

		Iterator_Detail operator++()
		{
			m_node = m_node->m_next;
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
	};

 public:
	using Iterator       = Iterator_Detail<false>;
	using Const_Iterator = Iterator_Detail<true>;

	Satellite m_satellite;
	Pointer   m_next;

	void initialise()
	{
		m_next = nullptr;
	}
};

} // namespace detail

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
	using Node               = detail::List_Node<Value_t, Allocator_Base>;
	using Node_Traits        = detail::Node_Traits<Node>;
	using Node_Allocator     = typename Node_Traits::Allocator;
	using Node_Pointer       = typename Node_Traits::Pointer;
	using Node_Const_Pointer = typename Node_Traits::Const_Pointer;

	using Alloc_Traits = Allocator_Traits<Allocator_Base<Value_t>>;

 public:
	using Allocator      = typename Alloc_Traits::Allocator;
	using Value          = typename Alloc_Traits::Value;
	using Pointer        = typename Alloc_Traits::Pointer;
	using Const_Pointer  = typename Alloc_Traits::Const_Pointer;
	using Iterator       = typename Node::Iterator;
	using Const_Iterator = typename Node::Const_Iterator;

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
			to = Node_Traits::create_node(m_allocator, value);

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
			to = Node_Traits::create_node(m_allocator, from->m_satellite);

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

	[[nodiscard]] Iterator begin()
	{
		return Iterator(m_head);
	}

	[[nodiscard]] Const_Iterator begin() const
	{
		return Const_Iterator(m_head);
	}

	[[nodiscard]] Iterator end()
	{
		return Iterator(nullptr);
	}

	[[nodiscard]] Const_Iterator end() const
	{
		return Const_Iterator(nullptr);
	}

	[[nodiscard]] Value &operator[](std::size_t index)
	{
		return at(index)->m_satellite;
	}

	[[nodiscard]] const Value &operator[](std::size_t index) const
	{
		return at(index)->m_satellite;
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
		return m_head->m_satellite;
	}

	/**
	 * @brief Gets the first element in the list. This is undefined
	 * behaviour if the list is empty
	 */
	[[nodiscard]] const Value &front() const
	{
		return m_head->m_satellite;
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
			Node_Traits::destroy_node(m_allocator, node);
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
		    Node_Traits::create_node(m_allocator, std::move(value));

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
			Node_Traits::destroy_node(m_allocator, remove);
			return;
		}

		Node_Pointer previous = at(index - 1);
		Node_Pointer remove   = previous->m_next;
		previous->m_next      = remove->m_next;
		Node_Traits::destroy_node(m_allocator, remove);
	}

	friend bool operator==(List const &lhs, List const &rhs) noexcept
	{
		Node_Pointer lhs_node = lhs.m_head;
		Node_Pointer rhs_node = rhs.m_head;
		while (lhs_node != nullptr && rhs_node != nullptr)
		{
			if (lhs_node->m_satellite != rhs_node->m_satellite)
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
