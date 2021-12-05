#ifndef DSA_DYNAMIC_ARRAY_HPP
#define DSA_DYNAMIC_ARRAY_HPP

#include <cstddef>
#include <memory>

namespace dsa
{

template<typename T, template<typename> typename Allocator_t = std::allocator>
class Dynamic_Array
{
 public:
	template<typename Allocator_Value>
	using Allocator_Base = Allocator_t<Allocator_Value>;

	using Value     = T;
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
		m_allocator.deallocate(m_array);
	}

	Dynamic_Array(const Dynamic_Array &darray)
	    : m_allocator(darray.m_allocator)
	    , m_size(darray.size())
	    , m_array(darray.m_allocator.allocate(darray.size()))
	{
		std::copy(darray.m_array, darray.m_array + darray.size(), m_array);
	}

	Dynamic_Array(Dynamic_Array &&darray) noexcept
	    : Dynamic_Array{ darray.m_allocator }
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
		return m_array[index];
	}

	[[nodiscard]] const Value &operator[](std::size_t index) const
	{
		return m_array[index];
	}

	[[nodiscard]] std::size_t size() const
	{
		return m_size;
	}

	void resize(std::size_t new_size)
	{
		Value *array = m_allocator.allocate(new_size);

		const std::size_t count = std::min(m_size, new_size);
		std::move(m_array, m_array + count, array);
		m_allocator.deallocate(m_array);

		m_array = array;
		m_size  = new_size;
	}

 private:
	Allocator m_allocator;

	std::size_t m_size  = 0;
	Value      *m_array = nullptr;
};

} // namespace dsa

#endif
