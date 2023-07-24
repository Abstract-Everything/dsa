#ifndef DSA_VECTOR_HPP
#define DSA_VECTOR_HPP

#include <dsa/allocator_traits.hpp>
#include <dsa/default_allocator.hpp>

#include <algorithm>
#include <memory>
#include <ostream>
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
template<typename Value_t, typename Allocator_t = Default_Allocator<Value_t>>
class Vector
{
 private:
	using Alloc_Traits = Allocator_Traits<Allocator_t>;

 public:
	using Allocator       = typename Alloc_Traits::Allocator;
	using Value           = typename Alloc_Traits::Value;
	using Reference       = typename Alloc_Traits::Reference;
	using Const_Reference = typename Alloc_Traits::Const_Reference;
	using Pointer         = typename Alloc_Traits::Pointer;
	using Const_Pointer   = typename Alloc_Traits::Const_Pointer;
	using Iterator        = Pointer;
	using Const_Iterator  = Const_Pointer;

	constexpr Allocator const &allocator() const {
		return m_allocator;
	}

	/**
	 * @brief Constucts an empty vector
	 */
	explicit Vector(Allocator allocator = Allocator())
	    : Vector(0, std::move(allocator)) {
	}

	/**
	 * @brief Constructs a vector of the given size whose elements are
	 * default initialised
	 */
	explicit Vector(std::size_t size, Allocator allocator)
	    : Vector(size, Value{}, std::move(allocator)) {
	}

	/**
	 * @brief Constructs a vector of the given size whose elements are
	 * initalised to the given value
	 */
	explicit Vector(
	    std::size_t    size,
	    Value_t const &value     = Value(),
	    Allocator      allocator = Allocator())
	    : m_allocator(std::move(allocator))
	    , m_size(size)
	    , m_capacity(size)
	    , m_storage(Alloc_Traits::allocate(m_allocator, size)) {
		std::uninitialized_fill(begin(), end(), value);
	}

	/**
	 * @brief Constructs an vector filled with the given values
	 */
	Vector(std::initializer_list<Value_t> values, Allocator allocator = Allocator())
	    : m_allocator(std::move(allocator))
	    , m_size(values.size())
	    , m_capacity(values.size())
	    , m_storage(Alloc_Traits::allocate(m_allocator, values.size())) {
		std::uninitialized_copy(
		    std::begin(values),
		    std::end(values),
		    begin());
	}

	~Vector() {
		if (data() == nullptr)
		{
			return;
		}

		std::destroy(begin(), end());
		Alloc_Traits::deallocate(m_allocator, m_storage, m_capacity);
	}

	Vector(Vector const &vector)
	    : m_allocator(
		Alloc_Traits::propogate_or_create_instance(vector.allocator()))
	    , m_size(vector.size())
	    , m_capacity(vector.capacity())
	    , m_storage(Alloc_Traits::allocate(m_allocator, m_capacity)) {
		std::uninitialized_copy(vector.begin(), vector.end(), begin());
	}

	Vector &operator=(Vector const &vector) {
		using std::swap;

		Vector copy(vector);
		swap(*this, copy);
		return *this;
	}

	Vector(Vector &&vector) noexcept
	    : m_allocator(std::move(vector.m_allocator))
	    , m_size(vector.size())
	    , m_capacity(vector.m_capacity)
	    , m_storage(vector.m_storage) {
		vector.m_storage = nullptr;
	}

	Vector &operator=(Vector &&vector) noexcept {
		using std::swap;

		swap(*this, vector);
		return *this;
	}

	friend void swap(Vector &lhs, Vector &rhs) noexcept {
		using std::swap;

		swap(lhs.m_allocator, rhs.m_allocator);
		swap(lhs.m_capacity, rhs.m_capacity);
		swap(lhs.m_storage, rhs.m_storage);
		swap(lhs.m_size, rhs.m_size);
	}

	/**
	 * @brief Checks if each element in both vectors is equal
	 */
	[[nodiscard]] friend bool operator==(
	    Vector const &lhs,
	    Vector const &rhs) noexcept {
		return lhs.size() == rhs.size()
		       && std::equal(lhs.begin(), lhs.end(), rhs.begin());
	}

	/**
	 * @brief Checks if any element in both vectors differs
	 */
	[[nodiscard]] friend bool operator!=(
	    Vector const &lhs,
	    Vector const &rhs) noexcept {
		return !(lhs == rhs);
	}

	/**
	 * @brief Gets the first element in the vector. This is undefined
	 * behaviour if the vector is empty
	 */
	[[nodiscard]] Reference front() {
		return *begin();
	}

	/**
	 * @brief Gets the first element in the vector. This is undefined
	 * behaviour if the vector is empty
	 */
	[[nodiscard]] Const_Reference front() const {
		return *begin();
	}

	/**
	 * @brief Gets the last element in the vector. This is undefined
	 * behaviour if the vector is empty
	 */
	[[nodiscard]] Reference back() {
		Pointer past_end = end();
		return *(--past_end);
	}

	/**
	 * @brief Gets the last element in the vector. This is undefined
	 * behaviour if the vector is empty
	 */
	[[nodiscard]] Const_Reference back() const {
		Const_Pointer past_end = end();
		return *(--past_end);
	}

	/**
	 * @brief Returns true if the vector contains no initialised elements.
	 * Note that the capacity may still be larger than zero even if the
	 * vector is empty
	 */
	[[nodiscard]] bool empty() const {
		return size() == 0;
	}

	/**
	 * @brief Returns the number of initialised elements that the vector
	 * holds
	 */
	[[nodiscard]] std::size_t size() const {
		return m_size;
	}

	/**
	 * @brief Returns the maximum number of initialised elements that the
	 * vector can hold without having to resize
	 */
	[[nodiscard]] std::size_t capacity() const {
		return m_capacity;
	}

	[[nodiscard]] Pointer begin() {
		return m_storage;
	}

	[[nodiscard]] Const_Pointer begin() const {
		return m_storage;
	}

	[[nodiscard]] Pointer end() {
		return begin() + m_size;
	}

	[[nodiscard]] Const_Pointer end() const {
		return begin() + m_size;
	}

	[[nodiscard]] Reference operator[](std::integral auto index) {
		return m_storage[index];
	}

	[[nodiscard]] Const_Reference operator[](std::integral auto index) const {
		return m_storage[index];
	}

	friend std::ostream &operator<<(
	    std::ostream &stream,
	    Vector const &vector) noexcept {
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
	[[nodiscard]] Pointer data() {
		return m_storage;
	}

	/**
	 * @brief Returns a pointer to the underlying memory array where the
	 * elements are held
	 */
	[[nodiscard]] Const_Pointer data() const {
		return m_storage;
	}

	/**
	 * @brief Clears all elements from the vector
	 */
	void clear() {
		std::destroy(begin(), end());
		m_size = 0;
	}

	/**
	 * @brief Inserts the given value at the end of the vector
	 */
	void append(Value value) {
		grow();
		Alloc_Traits::construct(m_allocator, end(), std::move(value));
		m_size++;
	}

	/**
	 * @brief Inserts the given value at the given index. The behaviour is
	 * undefined if the index is outside of the range: [0, size()]
	 */
	void insert(std::size_t index, Value value) {
		using std::swap;

		if (should_grow())
		{
			size_t  capacity = grow_size();
			Pointer storage =
			    Alloc_Traits::allocate(m_allocator, capacity);
			Pointer insert_point = storage + index;
			Pointer rest         = insert_point + 1;
			Alloc_Traits::construct(
			    m_allocator,
			    insert_point,
			    std::move(value));
			std::uninitialized_move(begin(), begin() + index, storage);
			std::uninitialized_move(begin() + index, end(), rest);
			Alloc_Traits::deallocate(m_allocator, m_storage, m_capacity);
			m_storage  = storage;
			m_capacity = capacity;
		}
		else
		{
			Pointer insert_point = begin() + index;
			std::uninitialized_move(
			    std::reverse_iterator(end() - 1),
			    std::reverse_iterator(insert_point - 1),
			    std::reverse_iterator(end()));

			Alloc_Traits::construct(
			    m_allocator,
			    insert_point,
			    std::move(value));
		}

		m_size++;
	}

	/**
	 * @brief Erases the value at the given index. The behaviour is
	 * undefined if the index is outside of the vector size.
	 */
	void erase(std::size_t index) {
		using std::swap;

		Pointer erasing = begin() + index;
		Alloc_Traits::destroy(m_allocator, erasing);

		if (should_shrink())
		{
			size_t  capacity = shrink_size();
			Pointer storage =
			    Alloc_Traits::allocate(m_allocator, capacity);
			std::uninitialized_move(begin(), erasing, storage);
			std::uninitialized_move(erasing + 1, end(), storage + index);
			Alloc_Traits::deallocate(m_allocator, m_storage, m_capacity);
			m_storage  = storage;
			m_capacity = capacity;
		}
		else
		{
			std::uninitialized_move(erasing + 1, end(), erasing);
		}
		m_size--;
	}

	/**
	 * @brief Shrinks the vector so that no memory is occuppied outside of
	 * the elements held
	 */
	void shrink_to_fit() {
		using std::swap;

		size_t  capacity = m_size;
		Pointer storage = Alloc_Traits::allocate(m_allocator, capacity);
		std::uninitialized_move(begin(), end(), storage);
		Alloc_Traits::deallocate(m_allocator, m_storage, m_capacity);
		m_storage  = storage;
		m_capacity = capacity;
	}

	/**
	 * @brief Resizes the vector to contain the given amount of elements.
	 * If the new size is larger than the old, new elements are default
	 * initialised
	 */
	void resize(std::size_t new_size) {
		if (new_size < size())
		{
			std::destroy(begin() + new_size, end());
			m_size = new_size;
			shrink_to_fit();
			return;
		}

		std::size_t items_to_insert = new_size - m_size;
		if (new_size > capacity())
		{
			reserve(new_size);
		}

		std::uninitialized_fill(end(), end() + items_to_insert, Value());
		m_size = new_size;
	}

	/**
	 * @brief Reallocates the underlying memory to hold at least the given
	 * number of elements without having to resize
	 */
	void reserve(std::size_t new_capacity) {
		using std::swap;

		if (capacity() >= new_capacity)
		{
			return;
		}

		Pointer storage =
		    Alloc_Traits::allocate(m_allocator, new_capacity);
		std::uninitialized_move(begin(), end(), storage);
		Alloc_Traits::deallocate(m_allocator, m_storage, m_capacity);
		m_storage  = storage;
		m_capacity = new_capacity;
	}

 private:
	Allocator   m_allocator;
	std::size_t m_size     = 0;
	std::size_t m_capacity = 0;
	Pointer     m_storage  = nullptr;

	/**
	 * @brief Returns whether the vector needs to grow in order to contain
	 * an additional element
	 */
	[[nodiscard]] bool should_grow() {
		return size() >= capacity();
	}

	void grow() {
		if (should_grow())
		{
			reserve(grow_size());
		}
	}

	[[nodiscard]] std::size_t grow_size() {
		return std::max(1ULL, 2ULL * capacity());
	}

	/**
	 * @brief Returns whether the vector should shrink after removing a
	 * single element from it
	 */
	[[nodiscard]] bool should_shrink() {
		return capacity() / 4ULL >= size() - 1;
	}

	[[nodiscard]] std::size_t shrink_size() {
		return capacity() / 2ULL;
	}
};

} // namespace dsa

#endif
