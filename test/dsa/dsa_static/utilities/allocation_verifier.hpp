#ifndef TEST_DSA_STATIC_ALLOCATION_VERIFIER_HPP
#define TEST_DSA_STATIC_ALLOCATION_VERIFIER_HPP

#include <dsa/allocator_traits.hpp>
#include <dsa/default_allocator.hpp>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <numeric>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace test
{

static constexpr const char *memory_leaked =
    "Allocated memory was not deallocated\n";

static constexpr const char *deallocating_unallocated_memory =
    "Deallocate was called on an address which does not point to "
    "allocated memory\n";

static constexpr const char *deallocating_count_mismatch =
    "Deallocate count does not match the allocation count\n";

namespace detail
{

template<typename T>
auto numeric_address(T *pointer) -> uintptr_t
{
	return reinterpret_cast<uintptr_t>(pointer);
}

class Alloc_Traits_Misuse : public std::runtime_error
{
 public:
	using std::runtime_error::runtime_error;
};

class Allocation_Block
{
 public:
	Allocation_Block()          = default;
	virtual ~Allocation_Block() = default;

	[[nodiscard]] virtual auto count() const -> size_t = 0;
	[[nodiscard]] virtual auto match_address(uintptr_t address) const
	    -> bool = 0;

	virtual void deallocate() = 0;
};

template<typename Type>
class Allocation_Block_Typed : public Allocation_Block
{
 public:
	Allocation_Block_Typed(Type *address, size_t count)
	    : Allocation_Block()
	    , m_address(address)
	    , m_count(count)
	{
	}

	~Allocation_Block_Typed() override = default;

	[[nodiscard]] auto count() const -> size_t override
	{
		return m_count;
	}

	[[nodiscard]] auto match_address(uintptr_t address) const -> bool override
	{
		return numeric_address(m_address) == address;
	}

	void deallocate() override
	{
		Allocator allocator;
		Alloc_Traits::deallocate(allocator, m_address, m_count);
	}

 private:
	using Allocator    = dsa::Default_Allocator<Type>;
	using Alloc_Traits = dsa::Allocator_Traits<Allocator>;

	Type  *m_address;
	size_t m_count;
};

} // namespace detail

class Allocation_Verifier
{
 public:
	~Allocation_Verifier() noexcept(false)
	{
		try
		{
			cleanup();
		}
		catch (...)
		{
			std::cerr << "Allocation_Verifier's destructor was "
				     "called with pending errors. This can "
				     "happen due to two reasons:\n"
				  << " - Either cleanup() was not manually "
				     "called before destruction.\n"
				  << " - Or more errors have been registered "
				     "since the call.\n";
			std::terminate();
		}
	}

	void cleanup()
	{
		for (auto &allocation : m_allocations)
		{
			allocation->deallocate();
			m_errors.insert(memory_leaked);
		}
		m_allocations.clear();

		if (m_errors.empty())
		{
			return;
		}

		auto errors = std::accumulate(
		    m_errors.begin(),
		    m_errors.end(),
		    std::string());
		m_errors.clear();

		throw detail::Alloc_Traits_Misuse(errors);
	}

	template<typename T>
	void on_allocate(T *address, size_t count)
	{
		m_allocations.emplace_back(
		    std::make_unique<detail::Allocation_Block_Typed<T>>(
			address,
			count));
	}

	template<typename T>
	void on_construct(T * /* address */)
	{
	}

	template<typename T>
	void on_copy_construct(T * /* destination */, T const * /* source */)
	{
	}

	template<typename T>
	void on_copy_assign(T * /* destination */, T const * /* source */)
	{
	}

	template<typename T>
	void on_underlying_value_copy_assign(T * /* destination */)
	{
	}

	template<typename T>
	void on_move_construct(T * /* destination */, T const * /* source */)
	{
	}

	template<typename T>
	void on_move_assign(T * /* destination */, T const * /* source */)
	{
	}

	template<typename T>
	void on_underlying_value_move_assign(T * /* destination */)
	{
	}

	template<typename T>
	void on_destroy(T * /* address */)
	{
	}

	template<typename T>
	auto before_deallocate(T *address, size_t count) -> bool
	{
		auto allocation = std::find_if(
		    m_allocations.begin(),
		    m_allocations.end(),
		    [&](Block const &block) {
			    return block->match_address(
				detail::numeric_address(address));
		    });

		if (allocation == m_allocations.end())
		{
			m_errors.insert(deallocating_unallocated_memory);
			return false;
		}

		if ((*allocation)->count() != count)
		{
			m_errors.insert(deallocating_count_mismatch);
			return false;
		}

		return true;
	}

	template<typename T>
	void on_deallocate(T *address, size_t /* count */)
	{
		auto allocation = std::find_if(
		    m_allocations.begin(),
		    m_allocations.end(),
		    [&](Block const &block) {
			    return block->match_address(
				detail::numeric_address(address));
		    });

		assert(
		  allocation != m_allocations.end()
		  && "The before_deallocate functions should verify if this operation is possible");
		m_allocations.erase(allocation);
	}

 private:
	using Block = std::unique_ptr<detail::Allocation_Block>;

	std::vector<Block>    m_allocations;
	std::set<std::string> m_errors;
};

} // namespace test

#endif
