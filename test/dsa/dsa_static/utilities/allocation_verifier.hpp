#ifndef TEST_DSA_STATIC_ALLOCATION_VERIFIER_HPP
#define TEST_DSA_STATIC_ALLOCATION_VERIFIER_HPP

#include <dsa/allocator_traits.hpp>
#include <dsa/default_allocator.hpp>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <numeric>
#include <optional>
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

static constexpr const char *constructing_unallocated_memory =
    "Construct was called on an address which does not point to "
    "allocated memory\n";

static constexpr const char *destroying_nonconstructed_memory =
    "Destroy was called on an address which does not point to "
    "constructed memory\n";

static constexpr const char *object_leaked =
    "Constructed memory was not destroyed\n";

static constexpr const char *assign_uninitialized_memory =
    "Assignment was made to unintialised memory\n";

static constexpr const char *assign_from_uninitialized_memory =
    "Assignment was made from an uninitialised value\n";

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
	enum class Allocation_Type
	{
		Owned,
		FromConstruct
	};

	Allocation_Block()          = default;
	virtual ~Allocation_Block() = default;

	[[nodiscard]] virtual auto count() const -> size_t = 0;

	[[nodiscard]] virtual auto match_address(uintptr_t address) const
	    -> bool = 0;

	[[nodiscard]] virtual auto contains(uintptr_t address) const -> bool = 0;

	[[nodiscard]] virtual auto is_initialised(uintptr_t address) const
	    -> bool = 0;

	[[nodiscard]] virtual auto owns_allocation() const -> bool = 0;

	virtual auto mark_constructed(uintptr_t address)
	    -> std::optional<std::string> = 0;

	virtual auto mark_assigned(uintptr_t address)
	    -> std::optional<std::string> = 0;

	virtual void mark_moved(uintptr_t address) = 0;

	virtual auto mark_destroyed(uintptr_t address)
	    -> std::optional<std::string> = 0;

	virtual auto cleanup() -> std::optional<std::string>    = 0;
	virtual auto deallocate() -> std::optional<std::string> = 0;
};

template<typename Type>
class Allocation_Block_Typed : public Allocation_Block
{
 public:
	Allocation_Block_Typed(Allocation_Type allocation_type, Type *address, size_t count)
	    : Allocation_Block()
	    , m_allocation_type(allocation_type)
	    , m_address(address)
	{
		m_state.resize(count);
	}

	~Allocation_Block_Typed() override = default;

	[[nodiscard]] auto count() const -> size_t override
	{
		return m_state.size();
	}

	[[nodiscard]] auto match_address(uintptr_t address) const -> bool override
	{
		return numeric_address(m_address) == address;
	}

	[[nodiscard]] auto contains(uintptr_t address) const -> bool override
	{
		return address >= numeric_address(m_address)
		       && address < numeric_address(m_address + count());
	}

	[[nodiscard]] auto is_initialised(uintptr_t address) const -> bool override
	{
		return element_state(address) == Element_State::Initialised;
	}

	[[nodiscard]] auto owns_allocation() const -> bool override
	{
		return m_allocation_type == Allocation_Type::Owned;
	}

	auto mark_constructed(uintptr_t address)
	    -> std::optional<std::string> override
	{
		Element_State &state = element_state(address);
		if (state == Element_State::Initialised)
		{
			return object_leaked;
		}
		state = Element_State::Initialised;
		return {};
	}

	auto mark_assigned(uintptr_t address) -> std::optional<std::string> override
	{
		Element_State &state = element_state(address);
		if (state != Element_State::Initialised)
		{
			return assign_uninitialized_memory;
		}
		return {};
	}

	void mark_moved(uintptr_t address) override
	{
		element_state(address) = Element_State::Moved;
	}

	auto mark_destroyed(uintptr_t address)
	    -> std::optional<std::string> override
	{
		Element_State &state = element_state(address);
		if (state == Element_State::Uninitialised)
		{
			return destroying_nonconstructed_memory;
		}
		state = Element_State::Uninitialised;
		return {};
	}

	auto cleanup() -> std::optional<std::string> override
	{
		if (!owns_allocation())
		{
			return {};
		}

		bool all_elements_destroyed = std::none_of(
		    m_state.begin(),
		    m_state.end(),
		    [](Element_State const &state)
		    { return state == Element_State::Initialised; });

		if (all_elements_destroyed)
		{
			return {};
		}

		Allocator allocator;
		for (size_t i = 0; i < count(); ++i)
		{
			if (m_state[i] == Element_State::Initialised)
			{
				Alloc_Traits::destroy(allocator, m_address + i);
			}
		}
		return object_leaked;
	}

	auto deallocate() -> std::optional<std::string> override
	{
		if (!owns_allocation())
		{
			return {};
		}

		Allocator allocator;
		Alloc_Traits::deallocate(allocator, m_address, count());
		return memory_leaked;
	}

 private:
	using Allocator    = dsa::Default_Allocator<Type>;
	using Alloc_Traits = dsa::Allocator_Traits<Allocator>;

	enum class Element_State
	{
		Uninitialised,
		Initialised,
		Moved
	};

	Allocation_Type            m_allocation_type;
	Type		           *m_address;
	std::vector<Element_State> m_state;

	auto element_state(uintptr_t address) -> Element_State &
	{
		assert(
		    contains(address)
		    && "Expected the address to be inside of the allocation");
		auto typed_address = reinterpret_cast<Type *>(address);
		auto index = static_cast<size_t>(typed_address - m_address);
		return m_state[index];
	}

	auto element_state(uintptr_t address) const -> Element_State const &
	{
		assert(
		    contains(address)
		    && "Expected the address to be inside of the allocation");
		auto typed_address = reinterpret_cast<Type *>(address);
		auto index = static_cast<size_t>(typed_address - m_address);
		return m_state[index];
	}
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
			add_error_if_any(allocation->cleanup());
			add_error_if_any(allocation->deallocate());
		}
		std::erase_if(
		    m_allocations,
		    [](Allocation const &allocation)
		    { return allocation->owns_allocation(); });

		if (m_errors.empty())
		{
			return;
		}

		std::string errors = std::accumulate(
		    m_errors.begin(),
		    m_errors.end(),
		    std::string(""));

		m_errors.clear();

		throw detail::Alloc_Traits_Misuse(errors);
	}

	template<typename T>
	void on_allocate(T *address, size_t count)
	{
		m_allocations.emplace_back(
		    std::make_unique<detail::Allocation_Block_Typed<T>>(
			detail::Allocation_Block_Typed<T>::Allocation_Type::Owned,
			address,
			count));
	}

	template<typename T>
	void on_construct(T *typed_address)
	{
		uintptr_t address    = detail::numeric_address(typed_address);
		auto      allocation = find_containing_allocation(address);
		if (allocation == m_allocations.end())
		{
			// If we do not find an allocation with this address we
			// assume that this is a stack variable. This is not
			// correct because we could be constructing on memory
			// which we are not aware of. However, due to the way
			// Memory_Monitor works it is the best we can do
			m_allocations.emplace_back(
			    std::make_unique<detail::Allocation_Block_Typed<T>>(
				detail::Allocation_Block_Typed<T>::Allocation_Type::FromConstruct,
				typed_address,
				1));
			allocation = m_allocations.end() - 1;
		}

		add_error_if_any((*allocation)->mark_constructed(address));
	}

	template<typename T>
	void on_copy_construct(T *destination, T const *source)
	{
		verify_constructed(detail::numeric_address(source));
		on_construct(destination);
	}

	template<typename T>
	void on_copy_assign(T *destination, T const *source)
	{
		verify_constructed(detail::numeric_address(source));
		on_assign(detail::numeric_address(destination));
	}

	template<typename T>
	void on_underlying_value_copy_assign(T *destination)
	{
		on_assign(detail::numeric_address(destination));
	}

	template<typename T>
	void on_move_construct(T *destination, T const *source)
	{
		verify_constructed(detail::numeric_address(source));
		on_construct(destination);
		on_moved(detail::numeric_address(source));
	}

	template<typename T>
	void on_move_assign(T *destination, T const *source)
	{
		verify_constructed(detail::numeric_address(source));
		on_assign(detail::numeric_address(destination));
		on_moved(detail::numeric_address(source));
	}

	template<typename T>
	void on_underlying_value_move_assign(T *destination)
	{
		on_assign(detail::numeric_address(destination));
	}

	template<typename T>
	void on_destroy(T *typed_address)
	{
		uintptr_t address    = detail::numeric_address(typed_address);
		auto      allocation = find_containing_allocation(address);
		if (allocation == m_allocations.end())
		{
			m_errors.insert(destroying_nonconstructed_memory);
			return;
		}

		add_error_if_any((*allocation)->mark_destroyed(address));

		// For stack values we treat destroy as a deallocate aswell.
		// Stack values will never get a cal to deallocate and the same
		// address can be used as soon as destroy is called. For example
		// in a loop that calls a function each iteration
		if (!(*allocation)->owns_allocation())
		{
			m_allocations.erase(allocation);
		}
	}

	template<typename T>
	auto before_deallocate(T *address, size_t count) -> bool
	{
		auto allocation = std::find_if(
		    m_allocations.begin(),
		    m_allocations.end(),
		    [&](Allocation const &block) {
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
		    [&](Allocation const &block) {
			    return block->match_address(
				detail::numeric_address(address));
		    });

		assert(
		  allocation != m_allocations.end()
		  && "The before_deallocate functions should verify if this operation is possible");

		add_error_if_any((*allocation)->cleanup());

		m_allocations.erase(allocation);
	}

 private:
	using Allocation  = std::unique_ptr<detail::Allocation_Block>;
	using Allocations = std::vector<Allocation>;

	Allocations           m_allocations;
	std::set<std::string> m_errors;

	auto find_containing_allocation(uintptr_t address) -> Allocations::iterator
	{
		return std::find_if(
		    m_allocations.begin(),
		    m_allocations.end(),
		    [&](Allocation const &block)
		    { return block->contains(address); });
	}

	void verify_constructed(uintptr_t address)
	{
		auto allocation = find_containing_allocation(address);
		if (allocation == m_allocations.end()
		    || !(*allocation)->is_initialised(address))
		{
			m_errors.insert(assign_from_uninitialized_memory);
			return;
		}
	}

	void on_assign(uintptr_t address)
	{
		auto allocation = find_containing_allocation(address);
		if (allocation == m_allocations.end())
		{
			m_errors.insert(assign_uninitialized_memory);
			return;
		}

		add_error_if_any((*allocation)->mark_assigned(address));
	}

	void on_moved(uintptr_t address)
	{
		auto allocation = find_containing_allocation(address);
		if (allocation == m_allocations.end())
		{
			return;
		}
		(*allocation)->mark_moved(address);
	}

	void add_error_if_any(std::optional<std::string> &&error)
	{
		if (error.has_value())
		{
			m_errors.emplace(std::move(error.value()));
		}
	}
};

} // namespace test

#endif
