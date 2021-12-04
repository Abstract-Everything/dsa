#ifndef DSA_VECTOR_HPP
#define DSA_VECTOR_HPP

#include "dsa/dynamic_array.hpp"

#include <algorithm>
#include <memory>

namespace dsa
{

// ToDo: Use iterators for indices
template<typename T, typename Allocator_t = std::allocator<T>>
class Vector
{
	using Allocator = Allocator_t;
	using Value     = T;

 public:
	explicit Vector(const Allocator &allocator = std::allocator<Value>{})
	    : m_storage(allocator)
	{
	}

	[[nodiscard]] Value &front()
	{
		return m_storage[0];
	}

	[[nodiscard]] const Value &front() const
	{
		return m_storage[0];
	}

	[[nodiscard]] Value &back()
	{
		return m_storage[m_end - 1];
	}

	[[nodiscard]] const Value &back() const
	{
		return m_storage[m_end - 1];
	}

	[[nodiscard]] bool empty() const
	{
		return m_end == 0;
	}

	[[nodiscard]] std::size_t size() const
	{
		return m_end;
	}

	[[nodiscard]] std::size_t capacity() const
	{
		return m_storage.size();
	}

	[[nodiscard]] Value &operator[](std::size_t index)
	{
		return m_storage[index];
	}

	[[nodiscard]] const Value &operator[](std::size_t index) const
	{
		return m_storage[index];
	}

	void clear()
	{
		m_end = 0;
		shrink_to_fit();
	}

	void append(Value value)
	{
		grow();
		m_storage[m_end] = std::move(value);
		m_end++;
	}

	void insert(std::size_t index, Value value)
	{
		if (should_grow())
		{
			Dynamic_Array<Value, Allocator> storage{
				next_grow_size(),
				m_storage.allocator()
			};
			std::move(&m_storage[0], &m_storage[index], &storage[0]);
			std::move(
			    &m_storage[index],
			    &m_storage[m_end],
			    &storage[index + 1]);
			std::swap(m_storage, storage);
		}
		else
		{
			std::move_backward(
			    &m_storage[index],
			    &m_storage[m_end],
			    &m_storage[m_end] + 1);
		}

		m_storage[index] = std::move(value);

		m_end++;
	}

	void erase(std::size_t index)
	{
		if (should_shrink())
		{
			Dynamic_Array<Value, Allocator> storage{
				next_shrink_size(),
				m_storage.allocator()
			};
			std::move(&m_storage[0], &m_storage[index], &storage[0]);
			std::move(
			    &m_storage[index + 1],
			    &m_storage[m_end],
			    &storage[index]);
			std::swap(m_storage, storage);
		}
		else
		{
			std::move(
			    &m_storage[index + 1],
			    &m_storage[m_end],
			    &m_storage[index]);
		}

		m_end--;
	}

	void shrink_to_fit()
	{
		m_storage.resize(m_end);
	}

	void resize(std::size_t size)
	{
		if (size < capacity())
		{
			m_end = size;
			shrink_to_fit();
		}
		else
		{
			m_storage.resize(size);
			for (std::size_t i = m_end; i < size; ++i)
			{
				m_storage[i] = Value{};
			}
			m_end = size;
		}
	}

 private:
	std::size_t                     m_end = 0;
	Dynamic_Array<Value, Allocator> m_storage;

	bool should_grow()
	{
		return size() >= m_storage.size();
	}

	void grow()
	{
		if (should_grow())
		{
			m_storage.resize(next_grow_size());
		}
	}

	std::size_t next_grow_size()
	{
		return std::max(1UL, 2U * capacity());
	}

	bool should_shrink()
	{
		return capacity() / 4U >= size() - 1;
	}

	std::size_t next_shrink_size()
	{
		return capacity() / 2U;
	}
};

} // namespace dsa

#endif
