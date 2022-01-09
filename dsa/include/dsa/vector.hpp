#ifndef DSA_VECTOR_HPP
#define DSA_VECTOR_HPP

#include "dsa/dynamic_array.hpp"

#include <algorithm>
#include <memory>

namespace dsa
{

/**
 * @brief Holds a set of contigious elements of the same type. The size of the
 * container is scaled automatically in order to achieve amortized constant
 * operations when working close to the back of the array.
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
    template<typename> typename Pointer_Base   = dsa::Weak_Pointer,
    template<typename> typename Allocator_Base = std::allocator>
class Vector
{
 public:
	using Value         = Value_t;
	using Pointer       = Pointer_Base<Value_t>;
	using Const_Pointer = Pointer_Base<const Value_t>;
	using Allocator     = Allocator_Base<Value>;

	using Storage = Dynamic_Array<Value, Pointer_Base, Allocator_Base>;

	/**
	 * @brief Constucts an empty vector
	 */
	explicit Vector(const Allocator &allocator = Allocator())
	    : m_storage(allocator)
	{
	}

	/**
	 * @brief Constructs a vector of the given size whose elements are
	 * default initalised
	 */
	explicit Vector(std::size_t size, const Allocator &allocator = Allocator())
	    : m_storage(size, Value(), allocator)
	    , m_end(size)
	{
	}

	/**
	 * @brief Constructs a vector of the given size whose elements are
	 * initalised to the given value
	 */
	explicit Vector(
	    std::size_t      size,
	    const Value     &value,
	    const Allocator &allocator = Allocator())
	    : m_storage(size, value, allocator)
	    , m_end(size)
	{
	}

	/**
	 * @brief Constructs an vector filled with the given values
	 */
	Vector(
	    std::initializer_list<Value> values,
	    const Allocator             &allocator = Allocator())
	    : m_storage(values, allocator)
	    , m_end(values.size())
	{
	}

	/**
	 * @brief Checks if each element in both vectors is equal
	 */
	[[nodiscard]] friend bool operator==(Vector const &lhs, Vector const &rhs) noexcept
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

	/**
	 * @brief Checks if any element in both vectors differs
	 */
	[[nodiscard]] friend bool operator!=(Vector const &lhs, Vector const &rhs) noexcept
	{
		return !(lhs == rhs);
	}

	/**
	 * @brief Gets the first element in the vector. This is undefined
	 * behaviour if the vector is empty
	 */
	[[nodiscard]] Value &front()
	{
		return m_storage[0];
	}

	/**
	 * @brief Gets the first element in the vector. This is undefined
	 * behaviour if the vector is empty
	 */
	[[nodiscard]] const Value &front() const
	{
		return m_storage[0];
	}

	/**
	 * @brief Gets the last element in the vector. This is undefined
	 * behaviour if the vector is empty
	 */
	[[nodiscard]] Value &back()
	{
		return m_storage[m_end - 1];
	}

	/**
	 * @brief Gets the last element in the vector. This is undefined
	 * behaviour if the vector is empty
	 */
	[[nodiscard]] const Value &back() const
	{
		return m_storage[m_end - 1];
	}

	/**
	 * @brief Returns true if the vector contains no initialised elements.
	 * Note that the capacity may still be larger than zero even if the
	 * vector is empty
	 */
	[[nodiscard]] bool empty() const
	{
		return m_end == 0;
	}

	/**
	 * @brief Returns the number of initialised elements that the vector
	 * holds
	 */
	[[nodiscard]] std::size_t size() const
	{
		return m_end;
	}

	/**
	 * @brief Returns the maximum number of initialised elements that the
	 * vector can hold without having to resize
	 */
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

	/**
	 * @brief Returns a pointer to the underlying memory array where the
	 * elements are held
	 */
	[[nodiscard]] Pointer data()
	{
		return m_storage.data();
	}

	/**
	 * @brief Returns a pointer to the underlying memory array where the
	 * elements are held
	 */
	[[nodiscard]] Const_Pointer data() const
	{
		return m_storage.data();
	}

	/**
	 * @brief Clears all elements from the vector
	 */
	void clear()
	{
		m_end = 0;
		shrink_to_fit();
	}

	/**
	 * @brief Inserts the given value at the end of the vector
	 */
	void append(Value value)
	{
		grow();
		m_storage[m_end] = std::move(value);
		m_end++;
	}

	/**
	 * @brief Inserts the given value at the given index. The behaviour is
	 * undefined if the index is outside of the range: [0, size()]
	 */
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

	/**
	 * @brief Erases the value at the given index. The behaviour is
	 * undefined if the index is outside of the vector size.
	 */
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

	/**
	 * @brief Shrinks the vector so that no memory is occuppied outside of
	 * the elements held
	 */
	void shrink_to_fit()
	{
		m_storage.resize(m_end);
	}

	/**
	 * @brief Resizes the vector to contain the given amount of elements.
	 * If the new size is larger than the old, new elements are default
	 * initialised
	 */
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
			m_end = size;
		}
	}

	/**
	 * @brief Reallocates the underlying memory to hold at least the given
	 * number of elements without having to resize
	 */
	void reserve(std::size_t size)
	{
		if (capacity() >= size)
		{
			return;
		}

		m_storage.resize(size);
	}

 private:
	std::size_t m_end = 0;
	Storage     m_storage;

	/**
	 * @brief Returns whether the vector needs to grow in order to contain
	 * an additional element
	 */
	[[nodiscard]] bool should_grow()
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

	[[nodiscard]] std::size_t grow_size()
	{
		return std::max(1ULL, 2ULL * capacity());
	}

	/**
	 * @brief Returns whether the vector should shrink after removing a
	 * single element from it
	 */
	[[nodiscard]] bool should_shrink()
	{
		return capacity() / 4ULL >= size() - 1;
	}

	[[nodiscard]] std::size_t shrink_size()
	{
		return capacity() / 2ULL;
	}
};

} // namespace dsa

#endif
