#ifndef DSA_HEAP_HPP
#define DSA_HEAP_HPP

#include <dsa/allocator_traits.hpp>
#include <dsa/default_allocator.hpp>
#include <dsa/vector.hpp>

#include <algorithm>

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
 * @tparam Comparator_t: The type of a comparator for which comparator(x,y)
 * holds
 * @tparam Allocator_Base: The type of allocator used for memory management
 *
 */
template<typename Value_t, typename Comparator_t = decltype(std::less{}), typename Allocator_t = Default_Allocator<Value_t>>
class Heap
{
 private:
	using Storage = dsa::Vector<Value_t, Allocator_t>;

 public:
	using Comparator      = Comparator_t;
	using Allocator       = typename Storage::Allocator;
	using Value           = typename Storage::Value;
	using Reference       = typename Storage::Reference;
	using Const_Reference = typename Storage::Const_Reference;
	using Pointer         = typename Storage::Pointer;
	using Const_Pointer   = typename Storage::Const_Pointer;

	/**
	 * @brief Constructs an empty heap
	 */
	Heap() = default;

	/**
	 * @brief Constructs a heap made up of the given elements
	 */
	Heap(std::initializer_list<Value_t> list, Comparator comparator = std::less{})
	    : m_comparator(std::move(comparator)) {
		m_storage.reserve(list.size());
		for (auto const &value : list)
		{
			push(value);
		}
	}

	friend void swap(Heap &lhs, Heap &rhs) {
		using std::swap;
		swap(lhs.m_storage, rhs.m_storage);
		swap(lhs.m_comparator, rhs.m_comparator);
	}

	/**
	 * @brief Returns the current number of elements in the heap
	 */
	[[nodiscard]] std::size_t size() const {
		return m_storage.size();
	}

	/**
	 * @brief Returns true if the heap contains no elements.
	 */
	[[nodiscard]] bool empty() const {
		return size() == 0ULL;
	}

	/**
	 * @brief Returns a reference to the underlying container storing the
	 * heap
	 */
	[[nodiscard]] Storage &storage() {
		return m_storage;
	}

	/**
	 * @brief Returns a const reference to the underlying container storing
	 * the heap
	 */
	[[nodiscard]] Storage const &storage() const {
		return m_storage;
	}

	/**
	 * @brief Returns a reference to the top element of the heap. This is
	 * undefined behaviour if the heap is empty
	 */
	Value &top() {
		return m_storage[0];
	}

	/**
	 * @brief Returns a const reference to the top element of the heap. This
	 * is undefined behaviour if the heap is empty
	 */
	Value const &top() const {
		return m_storage[0];
	}

	/**
	 * @brief Adds an element to the heap and does the work necessary to
	 * maintain the heap property
	 */
	void push(Value value) {
		using std::swap;

		std::size_t index = m_storage.size();
		m_storage.append(std::move(value));

		for (std::size_t parent = parent_index(index);
		     index != 0 && !m_comparator(m_storage[parent], m_storage[index]);
		     parent = parent_index(index))
		{
			swap(m_storage[parent], m_storage[index]);
			index = parent;
		}
	}

	/**
	 * @brief Removes the top element of the heap and does the work
	 * necessary to maintain the heap property. This is undefined behaviour
	 * if the heap is empty
	 */
	void pop() {
		using std::swap;

		if (size() == 1)
		{
			m_storage.clear();
			return;
		}

		const std::size_t last = m_storage.size() - 1;
		swap(m_storage[0], m_storage[last]);
		m_storage.erase(last);

		std::size_t parent = 0;
		for (std::size_t left = 1; left < m_storage.size(); left = child_index(parent))
		{
			const std::size_t right = left + 1;

			std::size_t smallest = parent;
			if (m_comparator(m_storage[left], m_storage[parent]))
			{
				smallest = left;
			}

			if (right < m_storage.size()
			    && m_comparator(m_storage[right], m_storage[smallest]))
			{
				smallest = right;
			}

			if (smallest == parent)
			{
				break;
			}

			swap(m_storage[parent], m_storage[smallest]);
			parent = smallest;
		}
	}

 private:
	Storage    m_storage;
	Comparator m_comparator;

	/**
	 * @brief Returns the index of the parent given an index of one of the
	 * children.
	 */
	[[nodiscard]] std::size_t parent_index(std::size_t index) const {
		return (index - 1) / 2;
	}

	/**
	 * @brief Returns the index of the first child given an index of the
	 * parent
	 */
	[[nodiscard]] std::size_t child_index(std::size_t index) const {
		return (index * 2) + 1;
	}
};

} // namespace dsa

#endif
