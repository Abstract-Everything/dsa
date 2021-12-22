#ifndef DSA_LIST_HPP
#define DSA_LIST_HPP

#include <dsa/weak_pointer.hpp>

#include <cstddef>
#include <memory>

namespace dsa
{

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

	explicit List(const Allocator &allocator = Allocator{})
	    : m_allocator(allocator)
	{
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
			to->value   = from.value;
			to->next    = nullptr;

			to   = &to->next;
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

	std::size_t size() const
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

	[[nodiscard]] Value &front()
	{
		return m_head->value;
	}

	[[nodiscard]] const Value &front() const
	{
		return m_head->value;
	}

	[[nodiscard]] bool empty() const
	{
		return m_head == nullptr;
	}

	void clear()
	{
		Pointer node = m_head;
		while (node != nullptr)
		{
			Pointer next = node->next;
			m_allocator.deallocate(node.get(), 1);
			node = next;
		}
	}

	void prepend(Value value)
	{
		insert(0, std::move(value));
	}

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

	void detatch_front(Value value)
	{
		erase(0, std::move(value));
	}

	void erase(std::size_t index)
	{
		if (index == 0)
		{
			Pointer remove = m_head;
			m_head         = m_head->next;
			m_allocator.deallocate(remove.get(), 1);
			return;
		}

		Pointer previous = at(index - 1);
		Pointer remove   = previous->next;
		previous->next   = remove->next;
		m_allocator.deallocate(remove.get(), 1);
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

	Pointer at(std::size_t index)
	{
		Pointer node = m_head;
		for (std::size_t i = 0; i < index; ++i)
		{
			node = node->next;
		}
		return node;
	}

	Pointer at(std::size_t index) const
	{
		Pointer node = m_head;
		for (std::size_t i = 0; i < index; ++i)
		{
			node = node->next;
		}
		return node;
	}
};

} // namespace dsa

#endif
