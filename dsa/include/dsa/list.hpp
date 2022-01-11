#ifndef DSA_LIST_HPP
#define DSA_LIST_HPP

#include <dsa/weak_pointer.hpp>

#include <cstddef>
#include <memory>

namespace dsa
{

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
template<
    typename Value_t,
    template<typename> typename Pointer_Base   = Weak_Pointer,
    template<typename> typename Allocator_Base = std::allocator>
class List
{
	class Node;
	using Allocator = Allocator_Base<Node>;
	using Pointer   = Pointer_Base<Node>;

 public:
	using Value = Value_t;

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
		Pointer *next = &m_head;
		for (auto value : values)
		{
			Pointer &to = *next;
			to          = m_allocator.allocate(1);
			to->value   = value;
			to->next    = nullptr;

			next = &to->next;
		}
	}

	~List()
	{
		clear();
	}

	List(List const &list) : m_allocator(list.m_allocator)
	{
		Pointer  from = list.m_head;
		Pointer *next = &m_head;
		while (from != nullptr)
		{
			Pointer &to = *next;
			to          = m_allocator.allocate(1);
			to->value   = from->value;
			to->next    = nullptr;

			next = &to->next;
			from = from->next;
		}
	}

	void swap(List &lhs, List &rhs)
	{
		std::swap(lhs.m_allocator, rhs.m_allocator);
		std::swap(lhs.m_head, rhs.m_head);
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
		return at(index)->value;
	}

	[[nodiscard]] const Value &operator[](std::size_t index) const
	{
		return at(index)->value;
	}

	/**
	 * @brief Gets the number of elements currently in the list.
	 * Note: This operation is not constant as the size is not cached
	 */
	[[nodiscard]] std::size_t size() const
	{
		std::size_t list_size = 0;
		Pointer     node      = m_head;
		while (node != nullptr)
		{
			node = node->next;
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
		return m_head->value;
	}

	/**
	 * @brief Gets the first element in the list. This is undefined
	 * behaviour if the list is empty
	 */
	[[nodiscard]] const Value &front() const
	{
		return m_head->value;
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
		Pointer node = m_head;
		while (node != nullptr)
		{
			Pointer next = node->next;
			destroy_node(node);
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
		Pointer node = m_allocator.allocate(1);
		node->value  = value;

		if (index == 0)
		{
			node->next = m_head;
			m_head     = node;
			return;
		}

		Pointer previous = at(index - 1);
		node->next       = previous->next;
		previous->next   = node;
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
			Pointer remove = m_head;
			m_head         = m_head->next;
			destroy_node(remove);
			return;
		}

		Pointer previous = at(index - 1);
		Pointer remove   = previous->next;
		previous->next   = remove->next;
		destroy_node(remove);
	}

	friend bool operator==(List const &lhs, List const &rhs) noexcept
	{
		Pointer lhs_node = lhs.m_head;
		Pointer rhs_node = rhs.m_head;
		while (lhs_node != nullptr && rhs_node != nullptr)
		{
			if (lhs_node->value != rhs_node->value)
			{
				return false;
			}

			lhs_node = lhs_node->next;
			rhs_node = rhs_node->next;
		}

		return lhs_node == nullptr && rhs_node == nullptr;
	}

	friend bool operator!=(List const &lhs, List const &rhs) noexcept
	{
		return !(lhs == rhs);
	}

 private:
	class Node
	{
	 public:
		Value   value;
		Pointer next;
	};

	Allocator m_allocator;

	Pointer m_head;

	/**
	 * @brief Gets the node at the given index. The behaviour is undefined
	 * if the index is outside of the list range
	 */
	Pointer at(std::size_t index)
	{
		Pointer node = m_head;
		for (std::size_t i = 0; i < index; ++i)
		{
			node = node->next;
		}
		return node;
	}

	/**
	 * @brief Gets the node at the given index. The behaviour is undefined
	 * if the index is outside of the list range
	 */
	Pointer at(std::size_t index) const
	{
		Pointer node = m_head;
		for (std::size_t i = 0; i < index; ++i)
		{
			node = node->next;
		}
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
};

} // namespace dsa

#endif
