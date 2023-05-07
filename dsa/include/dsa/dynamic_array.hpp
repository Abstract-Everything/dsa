#ifndef DSA_DYNAMIC_ARRAY_HPP
#define DSA_DYNAMIC_ARRAY_HPP

#include <dsa/allocator_traits.hpp>
#include <dsa/default_allocator.hpp>

#include <concepts>
#include <type_traits>
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
	using Allocator       = typename Alloc_Traits::Allocator;
	using Value           = typename Alloc_Traits::Value;
	using Reference       = typename Alloc_Traits::Reference;
	using Const_Reference = typename Alloc_Traits::Const_Reference;
	using Pointer         = typename Alloc_Traits::Pointer;
	using Const_Pointer   = typename Alloc_Traits::Const_Pointer;
	using Iterator        = Pointer;
	using Const_Iterator  = Const_Pointer;

	[[nodiscard]] constexpr const Allocator &allocator() const
	{
		return m_allocator;
	}

	/**
	 * @brief Constructs an empty array
	 */
	constexpr explicit Dynamic_Array(Allocator allocator = Allocator{})
	    : Dynamic_Array(0, std::move(allocator))
	{
	}

	/**
	 * @brief Constructs an array of the given size whose values are
	 * default initialised
	 */
	constexpr explicit Dynamic_Array(std::size_t size, Allocator allocator)
	    : Dynamic_Array(size, Value{}, std::move(allocator))
	{
	}

	/**
	 * @brief Constructs an array of the given size whose values are
	 * initialised to the given value
	 */
	constexpr explicit Dynamic_Array(
	    std::size_t    size,
	    const Value_t &value     = Value{},
	    Allocator      allocator = Allocator{})
	    : m_allocator(std::move(allocator))
	    , m_size(size)
	    , m_storage(Alloc_Traits::allocate(m_allocator, size))
	{
		std::uninitialized_fill(begin(), end(), value);
	}

	/**
	 * @brief Constructs an array filled with the given values
	 */
	constexpr Dynamic_Array(
	    std::initializer_list<Value_t> values,
	    Allocator                      allocator = Allocator{})
	    : m_allocator(std::move(allocator))
	    , m_size(values.size())
	    , m_storage(Alloc_Traits::allocate(m_allocator, m_size))
	{
		std::uninitialized_copy(
		    std::begin(values),
		    std::end(values),
		    m_storage);
	}

	constexpr ~Dynamic_Array()
	{
		if (data() == nullptr)
		{
			return;
		}

		std::destroy(begin(), end());
		Alloc_Traits::deallocate(m_allocator, m_storage, m_size);
	}

	constexpr Dynamic_Array(const Dynamic_Array &darray)
	    : m_allocator(Alloc_Traits::propogate_or_create_instance(darray.allocator()))
	    , m_size(darray.size())
	    , m_storage(Alloc_Traits::allocate(m_allocator, m_size))
	{
		std::uninitialized_copy(darray.begin(), darray.end(), begin());
	}

	constexpr Dynamic_Array &operator=(Dynamic_Array const &darray) noexcept
	{
		using std::swap;

		Dynamic_Array copy(darray);
		swap(*this, copy);
		return *this;
	}

	constexpr Dynamic_Array(Dynamic_Array &&darray) noexcept
		: m_allocator(std::move(darray.m_allocator))
		, m_size(darray.m_size)
		, m_storage(darray.m_storage)
	{
		darray.m_storage = nullptr;
	}

	constexpr Dynamic_Array &operator=(Dynamic_Array &&darray) noexcept
	{
		using std::swap;

		swap(*this, darray);
		return *this;
	}

	friend constexpr bool operator==(Dynamic_Array const &lhs, Dynamic_Array const &rhs) noexcept
	{
		return lhs.size() == rhs.size()
		       && std::equal(lhs.begin(), lhs.end(), rhs.begin());
	}

	friend constexpr bool operator!=(Dynamic_Array const &lhs, Dynamic_Array const &rhs) noexcept
	{
		return !(lhs == rhs);
	}

	friend constexpr void swap(Dynamic_Array &lhs, Dynamic_Array &rhs)
	{
		using std::swap;

		swap(lhs.m_allocator, rhs.m_allocator);
		swap(lhs.m_size, rhs.m_size);
		swap(lhs.m_storage, rhs.m_storage);
	}

	[[nodiscard]] constexpr Value &operator[](std::integral auto index)
	{
		return m_storage[index];
	}

	[[nodiscard]] constexpr const Value &operator[](std::integral auto index) const
	{
		return m_storage[index];
	}

	[[nodiscard]] constexpr Pointer begin()
	{
		return m_storage;
	}

	[[nodiscard]] constexpr Const_Pointer begin() const
	{
		return m_storage;
	}

	[[nodiscard]] constexpr Pointer end()
	{
		return m_storage + m_size;
	}

	[[nodiscard]] constexpr Const_Pointer end() const
	{
		return m_storage + m_size;
	}

	/**
	 * Returns a pointer to the allocated storage
	 */
	[[nodiscard]] constexpr Pointer data()
	{
		return m_storage;
	}

	/**
	 * Returns a pointer to the allocated storage
	 */
	[[nodiscard]] constexpr Const_Pointer data() const
	{
		return m_storage;
	}

	/**
	 * Returns the current number of elements that can be held
	 */
	[[nodiscard]] constexpr std::size_t size() const
	{
		return m_size;
	}

	/**
	 * Changes the size of the container. The first min(size, new_size)
	 * elements are moved from the previous memory, the rest are initialised
	 * to the given value
	 */
	constexpr void resize(std::size_t new_size, Value const &value = Value{})
	{
		using std::swap;

		const std::size_t count = std::min(m_size, new_size);

		Pointer storage = Alloc_Traits::allocate(m_allocator, new_size);
		std::uninitialized_fill(storage + count, storage + new_size, value);
		std::uninitialized_move(begin(), begin() + count, storage);
		std::destroy(begin() + count, end());
		Alloc_Traits::deallocate(m_allocator, m_storage, m_size);

		m_storage = storage;
		m_size = new_size;
	}

 private:
	Allocator m_allocator;
	size_t    m_size    = 0;
	Pointer   m_storage = nullptr;
};

} // namespace dsa

#endif
