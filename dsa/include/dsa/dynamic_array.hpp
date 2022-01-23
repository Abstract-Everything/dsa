#ifndef DSA_DYNAMIC_ARRAY_HPP
#define DSA_DYNAMIC_ARRAY_HPP

#include <dsa/allocator_traits.hpp>
#include <dsa/default_allocator.hpp>

#include <cstddef>
#include <memory>

namespace dsa
{

/**
 * @brief Represents a set of contigious elements of the same type whose size
 * can be changed at runtime.
 *
 * @ingroup containers
 *
 * @tparam Value_t: The type of element to store
 * @tparam Allocator_Base: The type of allocator used for memory management
 *
 */
template<typename Value_t, template<typename> typename Allocator_Base = Default_Allocator>
class Dynamic_Array
{
 private:
	using Alloc_Traits = Allocator_Traits<Allocator_Base<Value_t>>;

 public:
	using Allocator     = typename Alloc_Traits::Allocator;
	using Value         = typename Alloc_Traits::Value;
	using Pointer       = typename Alloc_Traits::Pointer;
	using Const_Pointer = typename Alloc_Traits::Const_Pointer;

	[[nodiscard]] const Allocator &allocator() const
	{
		return m_allocator;
	}

	/**
	 * @brief Constructs an empty array
	 */
	explicit Dynamic_Array(const Allocator &allocator = Allocator{})
	    : Dynamic_Array(0, allocator)
	{
	}

	/**
	 * @brief Constructs an array of the given size whose values are
	 * default initialised
	 */
	explicit Dynamic_Array(std::size_t size, const Allocator &allocator)
	    : Dynamic_Array(size, Value{}, allocator)
	{
	}

	/**
	 * @brief Constructs an array of the given size whose values are
	 * initialised to the given value
	 */
	explicit Dynamic_Array(
	    std::size_t      size,
	    const Value_t   &value     = {},
	    const Allocator &allocator = {})
	    : m_allocator(allocator)
	    , m_size(size)
	    , m_array(Alloc_Traits::allocate(m_allocator, size))
	{
		for (std::size_t i = 0; i < size; ++i)
		{
			Alloc_Traits::construct(m_allocator, m_array + i, value);
		}
	}

	/**
	 * @brief Constructs an array filled with the given values
	 */
	Dynamic_Array(
	    std::initializer_list<Value_t> values,
	    const Allocator               &allocator = Allocator{})
	    : Dynamic_Array(values.size(), allocator)
	{
		std::size_t index = 0;
		for (auto value : values)
		{
			this->operator[](index++) = value;
		}
	}

	~Dynamic_Array()
	{
		// This can be set to nullptr after a move
		if (m_array != nullptr)
		{
			for (std::size_t i = 0; i < m_size; ++i)
			{
				Alloc_Traits::destroy(m_allocator, m_array + i);
			}
			Alloc_Traits::deallocate(m_allocator, m_array, m_size);
		}
	}

	Dynamic_Array(const Dynamic_Array &darray)
	    : Dynamic_Array(darray.m_size, darray.allocator())
	{
		std::copy(darray.m_array, darray.m_array + darray.size(), m_array);
	}

	Dynamic_Array(Dynamic_Array &&darray) noexcept
	    : m_allocator{darray.m_allocator}
	    , m_array(nullptr)
	{
		swap(*this, darray);
	}

	friend void swap(Dynamic_Array &lhs, Dynamic_Array &rhs)
	{
		std::swap(lhs.m_size, rhs.m_size);
		std::swap(lhs.m_array, rhs.m_array);
	}

	Dynamic_Array &operator=(Dynamic_Array darray) noexcept
	{
		swap(*this, darray);
		return *this;
	}

	[[nodiscard]] Value &operator[](std::size_t index)
	{
		return m_array[index];
	}

	[[nodiscard]] const Value &operator[](std::size_t index) const
	{
		return m_array[index];
	}

	/**
	 * Returns a pointer to the allocated storage
	 */
	[[nodiscard]] Pointer data()
	{
		return m_array;
	}

	/**
	 * Returns a pointer to the allocated storage
	 */
	[[nodiscard]] Const_Pointer data() const
	{
		return Const_Pointer(m_array);
	}

	/**
	 * Returns the current number of elements that can be held
	 */
	[[nodiscard]] std::size_t size() const
	{
		return m_size;
	}

	/**
	 * Changes the size of the container. The first min(size, new_size)
	 * elements are moved from the previous memory, the rest are initialised
	 * to the given value
	 */
	void resize(std::size_t new_size, Value const &value = {})
	{
		Pointer array = Alloc_Traits::allocate(m_allocator, new_size);

		const std::size_t count = std::min(m_size, new_size);
		std::uninitialized_move(m_array, m_array + count, array);

		for (std::size_t i = m_size; i < new_size; ++i)
		{
			Alloc_Traits::construct(m_allocator, array + i, value);
		}

		Alloc_Traits::deallocate(m_allocator, m_array, m_size);

		m_array = array;
		m_size  = new_size;
	}

	friend bool operator==(Dynamic_Array const &lhs, Dynamic_Array const &rhs) noexcept
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

	friend bool operator!=(Dynamic_Array const &lhs, Dynamic_Array const &rhs) noexcept
	{
		return !(lhs == rhs);
	}

 private:
	// ToDo: If allocator instances are the same this member is not needed
	Allocator m_allocator;

	std::size_t m_size = 0;

	// We maintain the values in the array initialised, otherwise moving,
	// destroying and writing new values to the elements becomes
	// significantly more difficult to do well.
	//
	// Allocator::deallocate cannot be called with nullptr, thus creating an
	// allocation of 0 elements allows us to call deallocate without
	// explicitly checking for nullptr when resizing
	Pointer m_array = m_allocator.allocate(0);
};

} // namespace dsa

#endif
