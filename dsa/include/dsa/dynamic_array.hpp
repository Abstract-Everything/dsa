#ifndef DSA_DYNAMIC_ARRAY_HPP
#define DSA_DYNAMIC_ARRAY_HPP

#include <dsa/weak_pointer.hpp>

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
 * @tparam Pointer_Base: The type of pointer used to refer to memory
 * @tparam Allocator_Base: The type of allocator used for memory management
 *
 */
template<
    typename Value_t,
    template<typename> typename Pointer_Base   = dsa::Weak_Pointer,
    template<typename> typename Allocator_Base = std::allocator>
class Dynamic_Array
{
 public:
	using Value     = Value_t;
	using Pointer   = Pointer_Base<Value_t>;
	using Allocator = Allocator_Base<Value>;

	[[nodiscard]] const Allocator &allocator() const
	{
		return m_allocator;
	}

	/**
	 * @brief Constructs an empty array
	 */
	explicit Dynamic_Array(const Allocator &allocator = Allocator{})
	    : m_allocator(allocator)
	{
	}

	/**
	 * @brief Constructs an array of the given size filled with
	 * uninitialised values
	 */
	explicit Dynamic_Array(
	    std::size_t      size,
	    const Allocator &allocator = Allocator{})
	    : m_allocator(allocator)
	    , m_size(size)
	    , m_array(m_allocator.allocate(size))
	{
	}

	/**
	 * @brief Constructs an array filled with the given values
	 */
	Dynamic_Array(
	    std::initializer_list<Value> values,
	    const Allocator             &allocator = Allocator{})
	    : Dynamic_Array(values.size(), allocator)
	{
		std::size_t index = 0;
		for (auto value : values)
		{
			this->operator[](index++) = std::move(value);
		}
	}

	~Dynamic_Array()
	{
		// This can be set to nullptr after a move
		if (m_array != nullptr)
		{
			m_allocator.deallocate(m_array.get(), m_size);
		}
	}

	Dynamic_Array(const Dynamic_Array &darray)
	    : Dynamic_Array(darray.m_size, darray.allocator())
	{
		std::copy(
		    darray.m_array.get(),
		    darray.m_array.get() + darray.size(),
		    m_array.get());
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
		return m_array.get()[index];
	}

	[[nodiscard]] const Value &operator[](std::size_t index) const
	{
		return m_array.get()[index];
	}

	/**
	 * Returns a pointer to the allocated storage
	 */
	[[nodiscard]] Pointer data()
	{
		return m_array.get();
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
	 * elements are moved into the new allocation
	 */
	void resize(std::size_t new_size)
	{
		Pointer array = m_allocator.allocate(new_size);

		const std::size_t count = std::min(m_size, new_size);
		std::move(m_array.get(), m_array.get() + count, array.get());
		m_allocator.deallocate(m_array.get(), m_size);

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
	Allocator m_allocator;

	std::size_t m_size = 0;

	// Allocator::deallocate cannot be called with nullptr, thus creating an
	// allocation of 0 elements allows us to call deallocate without
	// explicitly checking for nullptr when resizing
	Pointer m_array = m_allocator.allocate(0);
};

} // namespace dsa

#endif
