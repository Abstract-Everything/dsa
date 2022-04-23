#ifndef DSA_HEAP_HPP
#define DSA_HEAP_HPP

#include <dsa/allocator_traits.hpp>
#include <dsa/default_allocator.hpp>
#include <dsa/vector.hpp>

namespace dsa
{

/**
 * @brief Represents a set of contiguous elements of the same type organised in
 * such a way as to allow fast access to the greatest element. The heap ensures
 * that the property comparator(root, child) holds for each element
 *
 * @ingroup containers
 *
 * @tparam Value_t: The type of element to store
 * @tparam Comparator: The type of a comparator for which comparator(x,y) holds
 * @tparam Allocator_Base: The type of allocator used for memory management
 *
 */
template<
    typename Value_t,
    typename Comparator                        = decltype(std::less{}),
    template<typename> typename Allocator_Base = Default_Allocator>
class Heap
{
 private:
	using Alloc_Traits = Allocator_Traits<Allocator_Base<Value_t>>;

 public:
	using Allocator     = typename Alloc_Traits::Allocator;
	using Value         = typename Alloc_Traits::Value;
	using Pointer       = typename Alloc_Traits::Pointer;
	using Const_Pointer = typename Alloc_Traits::Const_Pointer;

	using Storage = dsa::Vector<Value, Allocator_Base>;

	explicit Heap(Comparator comparator = std::less_equal{})
	    : m_comparator(std::move(comparator))
	{
	}

	/**
	 * @brief Returns the current number of elements in the heap
	 */
	[[nodiscard]] std::size_t size() const
	{
		return m_storage.size();
	}

	/**
	 * @brief Returns a reference to the underlying container storing the
	 * heap
	 */
	[[nodiscard]] Storage &storage()
	{
		return m_storage;
	}

	/**
	 * @brief Returns a const reference to the underlying container storing
	 * the heap
	 */
	[[nodiscard]] const Storage &storage() const
	{
		return m_storage;
	}

	/**
	 * @brief Adds an element to the heap and does the work necessary to
	 * maintain the heap property
	 */
	void push(Value value)
	{
		using std::swap;

		std::size_t index = m_storage.size();
		m_storage.append(std::move(value));

		for (std::size_t parent = parent_index(index);
		     index != 0
		     && !m_comparator(m_storage[parent], m_storage[index]);
		     parent = parent_index(index))
		{
			swap(m_storage[parent], m_storage[index]);
			index = parent;
		}
	}

 private:
	Storage    m_storage;
	Comparator m_comparator;

	/**
	 * @brief Returns the index of the parent given an index of one of the
	 * children.
	 */
	[[nodiscard]] std::size_t parent_index(std::size_t index) const
	{
		return (index - 1) / 2;
	}
};

} // namespace dsa

#endif
