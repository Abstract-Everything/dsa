#ifndef DSA_VECTOR_HPP
#define DSA_VECTOR_HPP

#include "dsa/dynamic_array.hpp"

#include <algorithm>
#include <memory>

namespace dsa
{

// ToDo: Use iterators for indices
template<
    typename Value_t,
    template<typename> typename Pointer_Base   = dsa::Weak_Pointer,
    template<typename> typename Allocator_Base = std::allocator>
class Vector
{
 public:
	using Value     = Value_t;
	using Allocator = Allocator_Base<Value>;

	using Storage = Dynamic_Array<Value, Pointer_Base, Allocator_Base>;

	explicit Vector(const Allocator &allocator = Allocator())
	    : m_storage(allocator)
	{
	}

	explicit Vector(
	    std::size_t      size,
	    const Value     &value     = Value(),
	    const Allocator &allocator = Allocator())
	    : m_storage(size, value, allocator)
	    , m_end(size)
	{
	}

	Vector(
	    std::initializer_list<Value> values,
	    const Allocator             &allocator = Allocator())
	    : m_storage(values, allocator)
	    , m_end(values.size())
	{
	}

	friend bool operator==(const Vector &lhs, const Vector &rhs)
	{
		if (lhs.size() != rhs.size())
		{
			return false;
		}

		for (std::size_t i = 0; i < lhs.size(); ++i)
		{
			if (lhs[i] != rhs[i])
			{
				return false;
			}
		}
		return true;
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
			Storage storage{grow_size(), m_storage.allocator()};
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
			Storage storage{shrink_size(), m_storage.allocator()};
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
	std::size_t m_end = 0;
	Storage     m_storage;

	bool should_grow()
	{
		return size() >= m_storage.size();
	}

	void grow()
	{
		if (should_grow())
		{
			m_storage.resize(grow_size());
		}
	}

	std::size_t grow_size()
	{
		return std::max(1ULL, 2ULL * capacity());
	}

	bool should_shrink()
	{
		return capacity() / 4ULL >= size() - 1;
	}

	std::size_t shrink_size()
	{
		return capacity() / 2ULL;
	}
};

} // namespace dsa

#endif
