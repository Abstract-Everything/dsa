#ifndef DSA_DYNAMIC_ARRAY_HPP
#define DSA_DYNAMIC_ARRAY_HPP

#include <dsa/weak_pointer.hpp>

#include <cstddef>
#include <memory>

namespace dsa
{

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

	[[nodiscard]] const Allocator &allocator()
	{
		return m_allocator;
	}

	explicit Dynamic_Array(
	    std::size_t      size,
	    const Allocator &allocator = Allocator{})
	    : Dynamic_Array(allocator)
	{
		resize(size);
	}

	explicit Dynamic_Array(const Allocator &allocator = Allocator{})
	    : m_allocator(allocator)
	{
	}

	~Dynamic_Array()
	{
		m_allocator.deallocate(m_array.get(), m_size);
	}

	Dynamic_Array(const Dynamic_Array &darray)
	    : m_allocator(darray.m_allocator)
	    , m_size(darray.size())
	    , m_array(darray.m_allocator.allocate(darray.size()))
	{
		std::copy(
		    darray.m_array.get(),
		    darray.m_array.get() + darray.size(),
		    m_array.get());
	}

	Dynamic_Array(Dynamic_Array &&darray) noexcept
	    : Dynamic_Array{darray.m_allocator}
	{
		swap(*this, darray);
	}

	friend void swap(Dynamic_Array &lhs, Dynamic_Array &rhs)
	{
		std::swap(lhs.m_allocator, rhs.m_allocator);
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

	[[nodiscard]] std::size_t size() const
	{
		return m_size;
	}

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

	std::size_t m_size  = 0;
	Pointer     m_array = nullptr;
};

} // namespace dsa

#endif
