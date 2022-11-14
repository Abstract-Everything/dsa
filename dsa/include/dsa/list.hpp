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
	using Alloc_Traits  = Allocator_Traits<Allocator_Base<List_Node>>;
	using Allocator     = typename Alloc_Traits::Allocator;
	using Pointer       = typename Alloc_Traits::Pointer;
	using Const_Pointer = typename Alloc_Traits::Const_Pointer;

	using Satellite_Traits = Allocator_Traits<Allocator_Base<Satellite_t>>;
	using Satellite_Value           = typename Satellite_Traits::Value;
	using Satellite_Reference       = typename Satellite_Traits::Reference;
	using Satellite_Const_Reference = typename Satellite_Traits::Const_Reference;
	using Satellite_Pointer         = typename Satellite_Traits::Pointer;
	using Satellite_Const_Pointer   = typename Satellite_Traits::Const_Pointer;

	template<bool Is_Const>
	class Iterator_Detail
	{
	 private:
		using Node_Pointer = std::conditional_t<
		    Is_Const,
		    typename List_Node::Const_Pointer,
		    typename List_Node::Pointer>;

		using Pointer =
		    std::conditional_t<Is_Const, Satellite_Const_Pointer, Satellite_Pointer>;

		using Reference =
		    std::conditional_t<Is_Const, Satellite_Const_Reference, Satellite_Reference>;

	 public:
		using iterator_category = std::forward_iterator_tag;
		using difference_type   = std::ptrdiff_t;
		using value_type        = Satellite_Value;
		using reference         = Reference;
		using pointer           = Pointer;

		explicit Iterator_Detail(Node_Pointer node) : m_node(node)
		{
		}

		Iterator_Detail operator++()
		{
			m_node = m_node->m_next;
			Iterator_Detail iterator(m_node);
			return iterator;
		}

		auto operator<=>(Iterator_Detail const &iterator) const
		    -> bool = default;

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

	template<typename... Arguments>
	explicit List_Node(Arguments &&...arguments)
	    : m_satellite(std::forward<Arguments>(arguments)...)
	    , m_next(nullptr)
	{
	}

	Satellite_Value m_satellite;
	Pointer         m_next = nullptr;
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
	using Node_Traits        = Allocator_Traits<Allocator_Base<Node>>;
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
	using Const_Pointer   = typename Alloc_Traits::Const_Pointer;
	using Iterator        = typename Node::Iterator;
	using Const_Iterator  = typename Node::Const_Iterator;

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
		Node_Pointer *owner = &m_head;
		for (auto const &value : values)
		{
			*owner = create_node(value);
			owner  = &(*owner)->m_next;
		}
	}

	~List()
	{
		clear();
	}

	List(List const &list) : m_allocator(list.m_allocator)
	{
		Node_Pointer *owner = &m_head;
		for (auto const &iterator : list)
		{
			Node_Pointer node = create_node(iterator);
			*owner            = node;
			owner             = &node->m_next;
		}
	}

	friend void swap(List &lhs, List &rhs)
	{
		using std::swap;
		swap(lhs.m_allocator, rhs.m_allocator);
		swap(lhs.m_head, rhs.m_head);
	}

	List(List &&list) noexcept
	    : m_allocator(std::move(list.m_allocator))
	    , m_head(std::move(list.m_head))
	{
		list.m_head = nullptr;
	}

	auto operator=(List list) noexcept -> List &
	{
		swap(*this, list);
		return *this;
	}

	[[nodiscard]] auto begin() -> Iterator
	{
		return Iterator(m_head);
	}

	[[nodiscard]] auto begin() const -> Const_Iterator
	{
		return Const_Iterator(m_head);
	}

	[[nodiscard]] auto end() -> Iterator
	{
		return Iterator(nullptr);
	}

	[[nodiscard]] auto end() const -> Const_Iterator
	{
		return Const_Iterator(nullptr);
	}

	/**
	 * @brief Gets the number of elements currently in the list.
	 * Note: This operation is not constant as the size is not cached
	 */
	[[nodiscard]] auto size() const -> std::size_t
	{
		size_t list_size = 0;
		for ([[maybe_unused]] auto const &iterator : *this)
		{
			list_size++;
		}
		return list_size;
	}

	/**
	 * @brief Gets the first element in the list. This is undefined
	 * behaviour if the list is empty
	 */
	[[nodiscard]] auto front() -> Reference
	{
		return m_head->m_satellite;
	}

	/**
	 * @brief Gets the first element in the list. This is undefined
	 * behaviour if the list is empty
	 */
	[[nodiscard]] auto front() const -> Const_Reference
	{
		return m_head->m_satellite;
	}

	/**
	 * @brief Returns true if the list contains no elements
	 */
	[[nodiscard]] auto empty() const -> bool
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
			destroy_node(node);
			node = next;
		}
		m_head = nullptr;
	}

	/**
	 * @brief Inserts the given value at the front of the list
	 */
	void prepend(Value_t value)
	{
		insert(0, std::move(value));
	}

	/**
	 * @brief Inserts the given value at the given index. The behaviour is
	 * undefined if the index is outside of the range: [0, size()]
	 */
	void insert(std::size_t index, Value_t value)
	{
		Node_Pointer node = create_node(std::move(value));

		Node_Pointer *owner = at(index);
		node->m_next        = *owner;
		*owner              = node;
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
		Node_Pointer *owner  = at(index);
		Node_Pointer  remove = *owner;
		*owner               = remove->m_next;
		destroy_node(remove);
	}

	friend bool operator==(List const &lhs, List const &rhs) noexcept
	{
		return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
	}

	friend bool operator!=(List const &lhs, List const &rhs) noexcept
	{
		return !(lhs == rhs);
	}

 private:
	Node_Allocator m_allocator;
	Node_Pointer   m_head = nullptr;

	template<typename... Arguments>
	auto create_node(Arguments &&...arguments) -> Node_Pointer
	{
		Node_Pointer node = Node_Traits::allocate(m_allocator, 1);
		Node_Traits::construct(
		    m_allocator,
		    node,
		    std::forward<Arguments>(arguments)...);
		return node;
	}

	void destroy_node(Node_Pointer node)
	{
		Node_Traits::destroy(m_allocator, node);
		Node_Traits::deallocate(m_allocator, node, 1);
	}

	/**
	 * @brief Gets the node at the given index. The behaviour is undefined
	 * if the index is outside of the list range
	 */
	auto at(std::size_t index) -> Node_Pointer *
	{
		Node_Pointer *node = &m_head;
		for (std::size_t i = 0; i < index; ++i)
		{
			node = &(*node)->m_next;
		}
		return node;
	}
};

} // namespace dsa

#endif
