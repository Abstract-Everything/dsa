#ifndef DSA_VECTOR_HPP
#define DSA_VECTOR_HPP

#include <dsa/allocator_traits.hpp>
#include <dsa/default_allocator.hpp>

#include <ostream>
#include <algorithm>
#include <memory>
#include <utility>

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
template<typename Value_t, template<typename> typename Allocator_Base = Default_Allocator>
class Vector
{
 private:
	using Alloc_Traits = Allocator_Traits<Allocator_Base<Value_t>>;

 public:
	using Allocator       = typename Alloc_Traits::Allocator;
	using Value           = typename Alloc_Traits::Value;
	using Pointer         = typename Alloc_Traits::Pointer;
	using Const_Pointer   = typename Alloc_Traits::Const_Pointer;
	using Reference       = typename Alloc_Traits::Reference;
	using Const_Reference = typename Alloc_Traits::Const_Reference;

	/**
	 * @brief Constucts an empty vector
	 */
	explicit Vector(const Allocator &allocator = Allocator())
	    : m_allocator(allocator)
	{
	}

	/**
	 * @brief Constructs a vector of the given size whose elements are
	 * default initialised
	 */
	explicit Vector(std::size_t size, const Allocator &allocator = Allocator())
	    : Vector(size, Value(), allocator)
	{
	}

	/**
	 * @brief Constructs a vector of the given size whose elements are
	 * initalised to the given value
	 */
	explicit Vector(
	    std::size_t      size,
	    const Value_t   &value,
	    const Allocator &allocator = Allocator())
	    : m_allocator(allocator)
	    , m_storage(Alloc_Traits::allocate(m_allocator, size))
	    , m_size(size)
	    , m_capacity(size)
	{
		for (std::size_t i = 0; i < size; ++i)
		{
			Alloc_Traits::construct(m_allocator, m_storage + i, value);
		}
	}

	/**
	 * @brief Constructs an vector filled with the given values
	 */
	Vector(
	    std::initializer_list<Value_t> values,
	    const Allocator               &allocator = Allocator())
	    : m_allocator(allocator)
	    , m_storage(Alloc_Traits::allocate(m_allocator, values.size()))
	    , m_size(values.size())
	    , m_capacity(values.size())
	{
		for (auto [value, it] = std::make_pair(std::as_const(values).begin(), begin());
		     value != values.end();
		     ++value, ++it)
		{
			Alloc_Traits::construct(m_allocator, it, *value);
		}
	}

	~Vector()
	{
		for (Reference value : *this)
		{
			Alloc_Traits::destroy(m_allocator, &value);
		}
		Alloc_Traits::deallocate(m_allocator, m_storage, capacity());
	}

	Vector(const Vector &vector)
	    : m_allocator(vector.m_allocator)
	    , m_storage(Alloc_Traits::allocate(m_allocator, vector.capacity()))
	    , m_size(vector.size())
	    , m_capacity(vector.capacity())
	{
		std::uninitialized_copy(vector.begin(), vector.end(), m_storage);
	}

	Vector(Vector &&vector) noexcept
	    : m_allocator(std::move(vector.m_allocator))
	    , m_storage(std::move(vector.m_storage))
	    , m_size(std::move(vector.m_size))
	    , m_capacity(std::move(vector.m_capacity))
	{
		vector.m_storage = Alloc_Traits::allocate(vector.m_allocator, 0);
		vector.m_capacity = 0;
	}

	friend void swap(Vector &lhs, Vector &rhs)
	{
		using std::swap;

		swap(lhs.m_allocator, rhs.m_allocator);
		swap(lhs.m_storage, rhs.m_storage);
		swap(lhs.m_size, rhs.m_size);
		swap(lhs.m_capacity, rhs.m_capacity);
	}

	Vector &operator=(Vector vector)
	{
		swap(*this, vector);
		return *this;
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
	[[nodiscard]] Reference front()
	{
		return *begin();
	}

	/**
	 * @brief Gets the first element in the vector. This is undefined
	 * behaviour if the vector is empty
	 */
	[[nodiscard]] Const_Reference front() const
	{
		return *begin();
	}

	/**
	 * @brief Gets the last element in the vector. This is undefined
	 * behaviour if the vector is empty
	 */
	[[nodiscard]] Reference back()
	{
		Pointer past_end = end();
		return *(--past_end);
	}

	/**
	 * @brief Gets the last element in the vector. This is undefined
	 * behaviour if the vector is empty
	 */
	[[nodiscard]] Const_Reference back() const
	{
		Const_Pointer past_end = end();
		return *(--past_end);
	}

	/**
	 * @brief Returns true if the vector contains no initialised elements.
	 * Note that the capacity may still be larger than zero even if the
	 * vector is empty
	 */
	[[nodiscard]] bool empty() const
	{
		return size() == 0;
	}

	/**
	 * @brief Returns the number of initialised elements that the vector
	 * holds
	 */
	[[nodiscard]] std::size_t size() const
	{
		return m_size;
	}

	/**
	 * @brief Returns the maximum number of initialised elements that the
	 * vector can hold without having to resize
	 */
	[[nodiscard]] std::size_t capacity() const
	{
		return m_capacity;
	}

	[[nodiscard]] Pointer begin()
	{
		return m_storage;
	}

	[[nodiscard]] Const_Pointer begin() const
	{
		return m_storage;
	}

	[[nodiscard]] Pointer end()
	{
		return &m_storage[m_size];
	}

	[[nodiscard]] Const_Pointer end() const
	{
		return &m_storage[m_size];
	}

	[[nodiscard]] Reference operator[](std::size_t index)
	{
		return *(begin() + index);
	}

	[[nodiscard]] Const_Reference operator[](std::size_t index) const
	{
		return *(begin() + index);
	}

	friend std::ostream& operator<<(std::ostream &stream, Vector const& vector) noexcept
	{
		stream << "[";
		for (Const_Pointer it = vector.begin(); it != vector.end(); ++it)
		{
			if (it != vector.begin())
			{
				stream << ", ";
			}
			stream << *it;
		}
		stream << "]";
		return stream;
	}

	/**
	 * @brief Returns a pointer to the underlying memory array where the
	 * elements are held
	 */
	[[nodiscard]] Pointer data()
	{
		return m_storage;
	}

	/**
	 * @brief Returns a pointer to the underlying memory array where the
	 * elements are held
	 */
	[[nodiscard]] Const_Pointer data() const
	{
		return m_storage;
	}

	/**
	 * @brief Clears all elements from the vector
	 */
	void clear()
	{
		for (Reference value : *this)
		{
			Alloc_Traits::destroy(m_allocator, &value);
		}
		m_size = 0;
	}

	/**
	 * @brief Inserts the given value at the end of the vector
	 */
	void append(Value value)
	{
		grow();
		Alloc_Traits::construct(m_allocator, m_storage + m_size, value);
		m_size++;
	}

	/**
	 * @brief Inserts the given value at the given index. The behaviour is
	 * undefined if the index is outside of the range: [0, size()]
	 */
	void insert(std::size_t index, Value value)
	{
		using std::swap;

		Pointer moving = begin() + index;
		if (should_grow())
		{
			std::size_t capacity = grow_size();
			Pointer     storage =
			    Alloc_Traits::allocate(m_allocator, capacity);
			std::uninitialized_move(begin(), moving, storage);
			std::uninitialized_move(moving, end(), storage + (index + 1));

			Alloc_Traits::deallocate(m_allocator, m_storage, m_capacity);
			swap(m_storage, storage);
			m_capacity = capacity;
			moving = begin() + index;
		}
		else
		{
			for (Pointer last = &back(); last != moving - 1; --last)
			{
				std::uninitialized_move(last, last + 1, last + 1);
			}
		}

		Alloc_Traits::construct(m_allocator, moving, std::move(value));
		m_size++;
	}

	/**
	 * @brief Erases the value at the given index. The behaviour is
	 * undefined if the index is outside of the vector size.
	 */
	void erase(std::size_t index)
	{
		using std::swap;

		Pointer erasing = &m_storage[index];
		Alloc_Traits::destroy(m_allocator, erasing);
		if (should_shrink())
		{
			std::size_t capacity = shrink_size();
			Pointer     storage =
			    Alloc_Traits::allocate(m_allocator, capacity);
			std::uninitialized_move(begin(), erasing, &storage[0]);
			std::uninitialized_move(erasing + 1, end(), &storage[index]);

			Alloc_Traits::deallocate(m_allocator, m_storage, m_capacity);
			swap(m_storage, storage);
			m_capacity = capacity;
		}
		else
		{
			for (Pointer i = erasing + 1; i != end(); ++i)
			{
				std::uninitialized_move(i, i + 1, i + (-1));
			}
		}

		m_size--;
	}

	/**
	 * @brief Shrinks the vector so that no memory is occuppied outside of
	 * the elements held
	 */
	void shrink_to_fit()
	{
		using std::swap;

		Pointer storage = Alloc_Traits::allocate(m_allocator, m_size);
		std::uninitialized_move(begin(), end(), storage);

		Alloc_Traits::deallocate(m_allocator, m_storage, m_capacity);
		swap(m_storage, storage);
		m_capacity = m_size;
	}

	/**
	 * @brief Resizes the vector to contain the given amount of elements.
	 * If the new size is larger than the old, new elements are default
	 * initialised
	 */
	void resize(std::size_t new_size)
	{
		if (new_size < capacity())
		{
			if (new_size < size())
			{
				Pointer last = begin() + new_size;
				for (Pointer i = last; i != end(); ++i)
				{
					Alloc_Traits::destroy(m_allocator, i);
				}
				m_size = new_size;
				shrink_to_fit();
			}
			else
			{
				std::size_t items_to_insert = new_size - m_size;
				for (std::size_t i = 0; i < items_to_insert; ++i)
				{
					append(Value());
				}
			}
		}
		else
		{
			reserve(new_size);
			for (Pointer i = end(); i != begin() + new_size; ++i)
			{
				Alloc_Traits::construct(m_allocator, i, Value());
			}
			m_size = new_size;
		}
	}

	/**
	 * @brief Reallocates the underlying memory to hold at least the given
	 * number of elements without having to resize
	 */
	void reserve(std::size_t new_capacity)
	{
		using std::swap;

		if (capacity() >= new_capacity)
		{
			return;
		}

		Pointer storage =
		    Alloc_Traits::allocate(m_allocator, new_capacity);
		std::uninitialized_move(begin(), end(), storage);

		Alloc_Traits::deallocate(m_allocator, m_storage, m_capacity);
		swap(m_storage, storage);
		m_capacity = new_capacity;
	}

 private:
	// ToDo: If allocator instances are the same this member is not needed
	Allocator   m_allocator;
	Pointer     m_storage  = Alloc_Traits::allocate(m_allocator, 0);
	std::size_t m_size     = 0;
	std::size_t m_capacity = 0;

	/**
	 * @brief Returns whether the vector needs to grow in order to contain
	 * an additional element
	 */
	[[nodiscard]] bool should_grow()
	{
		return size() >= capacity();
	}

	void grow()
	{
		if (should_grow())
		{
			reserve(grow_size());
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
