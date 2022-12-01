#ifndef TEST_DSA_STATIC_ALLOCATION_VERIFIER_HPP
#define TEST_DSA_STATIC_ALLOCATION_VERIFIER_HPP

#include <dsa/allocator_traits.hpp>
#include <dsa/default_allocator.hpp>
#include <dsa/memory_monitor.hpp>

#include <algorithm>
#include <any>
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
    "Assignment was made to uninitialised memory\n";

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

class Allocation_Element
{
 public:
	virtual ~Allocation_Element() = default;

	[[nodiscard]] auto initialised() const -> bool
	{
		return m_state == State::Initialised;
	}

	template<typename T>
	auto process_source_event(dsa::Object_Event<T> event)
	    -> std::optional<std::string>
	{
		if ((event.moving() || event.copying()) && !initialised())
		{
			return assign_from_uninitialized_memory;
		}

		if (event.moving())
		{
			m_state = State::Moved;
		}

		return {};
	}

	template<typename T>
	auto process_destination_event(dsa::Object_Event<T> event)
	    -> std::optional<std::string>
	{
		switch (event.type())
		{
		case dsa::Object_Event_Type::Before_Construct:
			return start_construction(event.destination());

		case dsa::Object_Event_Type::Construct:
		case dsa::Object_Event_Type::Copy_Construct:
		case dsa::Object_Event_Type::Move_Construct:
			return construct(event.destination());

		case dsa::Object_Event_Type::Copy_Assign:
		case dsa::Object_Event_Type::Underlying_Copy_Assign:
		case dsa::Object_Event_Type::Move_Assign:
		case dsa::Object_Event_Type::Underlying_Move_Assign:
			if (m_state != State::Initialised
			    && m_state != State::Moved)
			{
				return assign_uninitialized_memory;
			}

			if (m_state == State::Moved)
			{
				m_state = State::Initialised;
			}
			break;

		case dsa::Object_Event_Type::Destroy:
			return destroy(event.destination());
		}
		return {};
	}

 protected:
	Allocation_Element() = default;

	[[nodiscard]] virtual auto match_address(std::any const &address) const
	    -> bool = 0;

	[[nodiscard]] virtual auto contains_address(uintptr_t address) const
	    -> bool = 0;

 private:
	enum class State
	{
		/// Memory has been allocated for this element its constructor
		/// has not been called
		Uninitialised,
		/// We are currently running the constructor of this element.
		/// This state might have multiple calls if there are multiple
		/// fields in the structure.
		Constructing,
		/// We have constructed this element, it should be in a valid
		/// state
		Initialised,
		/// A move was executed on this object
		Moved
	};

	State m_state = State::Uninitialised;
	std::vector<std::unique_ptr<Allocation_Element>> m_fields;

	template<typename T>
	auto start_construction(T *address) -> std::optional<std::string>;

	template<typename T>
	auto construct(T *address) -> std::optional<std::string>
	{
		assert(contains_address(numeric_address(address)));
		if (match_address(address))
		{
			assert(m_state == State::Constructing);
			m_state = State::Initialised;
			return {};
		}

		auto element = std::find_if(
		    m_fields.begin(),
		    m_fields.end(),
		    [&](std::unique_ptr<Allocation_Element> const &element) {
			    return element->contains_address(
				numeric_address(address));
		    });
		assert(element != m_fields.end());
		(*element)->construct(address);
		return {};
	}

	template<typename T>
	auto destroy(T *address) -> std::optional<std::string>
	{
		assert(contains_address(numeric_address(address)));
		if (match_address(address))
		{
			assert(m_state != State::Constructing);
			if (m_state == State::Uninitialised)
			{
				return destroying_nonconstructed_memory;
			}
			m_state = State::Uninitialised;
			return {};
		}

		auto element = std::find_if(
		    m_fields.begin(),
		    m_fields.end(),
		    [&](std::unique_ptr<Allocation_Element> const &element) {
			    return element->contains_address(
				numeric_address(address));
		    });
		assert(element != m_fields.end());
		(*element)->destroy(address);
		return {};
	}
};

// Merge this with allocation block
template<typename T>
class Allocation_Element_Typed : public Allocation_Element
{
 public:
	explicit Allocation_Element_Typed(T *address)
	    : Allocation_Element()
	    , m_address(address)
	{
	}

	[[nodiscard]] auto match_address(std::any const &address) const
	    -> bool override
	{
		if (auto *typed_address = std::any_cast<T *>(&address))
		{
			return *typed_address == m_address;
		}

		if (auto *typed_address = std::any_cast<T const *>(&address))
		{
			return *typed_address == m_address;
		}
		return false;
	}

	[[nodiscard]] auto contains_address(uintptr_t address) const
	    -> bool override
	{
		uintptr_t base = numeric_address(m_address);
		return address >= base && address < base + sizeof(*m_address);
	}

 private:
	T *m_address;
};

template<typename T>
auto Allocation_Element::start_construction(T *address)
    -> std::optional<std::string>
{
	assert(contains_address(numeric_address(address)));
	if (match_address(address))
	{
		State old_state = m_state;
		m_state         = State::Constructing;
		return old_state != State::Uninitialised && old_state != State::Moved
			   ? object_leaked
			   : std::optional<std::string>{};
	}

	auto element = std::find_if(
	    m_fields.begin(),
	    m_fields.end(),
	    [&](std::unique_ptr<Allocation_Element> const &element)
	    { return element->contains_address(numeric_address(address)); });
	if (element == m_fields.end())
	{
		auto pointer =
		    std::make_unique<Allocation_Element_Typed<T>>(address);
		m_fields.emplace_back(std::move(pointer));
		element = m_fields.end() - 1;
	}

	(*element)->start_construction(address);
	return {};
}

enum class Allocation_Type
{
	Owned,
	FromConstruct
};

class Allocation_Block
{
 public:
	Allocation_Block()          = default;
	virtual ~Allocation_Block() = default;

	[[nodiscard]] virtual auto count() const -> size_t = 0;

	[[nodiscard]] virtual auto match_address(uintptr_t address) const
	    -> bool = 0;

	[[nodiscard]] virtual auto contains(uintptr_t address) const -> bool = 0;

	[[nodiscard]] virtual auto owns_allocation() const -> bool = 0;

	template<typename T>
	auto process_source_event(dsa::Object_Event<T> event)
	    -> std::optional<std::string>
	{
		return element(numeric_address(event.source()))
		    .process_source_event(event);
	}

	template<typename T>
	auto process_destination_event(dsa::Object_Event<T> event)
	    -> std::optional<std::string>
	{
		return element(numeric_address(event.destination()))
		    .process_destination_event(event);
	}

	virtual auto cleanup() -> std::optional<std::string>    = 0;
	virtual auto deallocate() -> std::optional<std::string> = 0;

 private:
	[[nodiscard]] virtual auto element(uintptr_t address)
	    -> Allocation_Element & = 0;

	[[nodiscard]] virtual auto element(uintptr_t address) const
	    -> Allocation_Element const & = 0;
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
		m_elements.reserve(count);
		for (size_t i = 0; i < count; ++i)
		{
			m_elements.emplace_back(
			    std::make_unique<detail::Allocation_Element_Typed<Type>>(
				address + i));
		}
	}

	~Allocation_Block_Typed() override = default;

	[[nodiscard]] auto count() const -> size_t override
	{
		return m_elements.size();
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

	[[nodiscard]] auto owns_allocation() const -> bool override
	{
		return m_allocation_type == Allocation_Type::Owned;
	}

	auto cleanup() -> std::optional<std::string> override
	{
		if (!owns_allocation())
		{
			return {};
		}

		bool all_elements_destroyed = std::none_of(
		    m_elements.begin(),
		    m_elements.end(),
		    [](Element const &element)
		    { return element->initialised(); });
		if (all_elements_destroyed)
		{
			return {};
		}

		Allocator allocator;
		for (size_t i = 0; i < count(); ++i)
		{
			if (m_elements[i]->initialised())
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
	using Element      = std::unique_ptr<Allocation_Element>;

	Allocation_Type      m_allocation_type;
	Type		    *m_address;
	std::vector<Element> m_elements;

	auto element_index(uintptr_t address) const -> size_t
	{
		assert(
		    contains(address)
		    && "Expected the address to be inside of the allocation");

		auto base_address = numeric_address(m_address);
		auto relative_address = address - base_address;
		return relative_address / sizeof(Type);
	}

	[[nodiscard]] auto element(uintptr_t address)
	    -> Allocation_Element & override
	{
		return *m_elements[element_index(address)];
	}

	[[nodiscard]] auto element(uintptr_t address) const
	    -> Allocation_Element const & override
	{
		return *m_elements[element_index(address)];
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

	static auto instance() -> std::unique_ptr<Allocation_Verifier> &
	{
		static std::unique_ptr<Allocation_Verifier> instance = nullptr;
		return instance;
	}

	template<typename T>
	static auto before_deallocate(dsa::Allocation_Event<T> event) -> bool
	{
		assert(event.type() == dsa::Allocation_Event_Type::Deallocate);
		return instance()->before_deallocate_impl(
		    event.address(),
		    event.count());
	}

	template<typename T>
	static void process_allocation_event(dsa::Allocation_Event<T> event)
	{
		switch (event.type())
		{
		case dsa::Allocation_Event_Type::Allocate:
			instance()->add_allocation(
			    detail::Allocation_Type::Owned,
			    event.address(),
			    event.count());
			break;

		case dsa::Allocation_Event_Type::Deallocate:
			instance()->on_deallocate(event.address());
			break;
		}
	}

	template<typename T>
	static void process_object_event(dsa::Object_Event<T> event)
	{
		instance()->process_object_event_impl(event);
	}

 private:
	using Allocation  = std::unique_ptr<detail::Allocation_Block>;
	using Allocations = std::vector<Allocation>;

	Allocations           m_allocations;
	std::set<std::string> m_errors;

	template<typename T>
	auto find_containing_allocation(T const *address) -> Allocations::iterator
	{
		return std::find_if(
		    m_allocations.begin(),
		    m_allocations.end(),
		    [&](Allocation const &block)
		    { return block->contains(detail::numeric_address(address)); });
	}

	template<typename T>
	void process_object_event_impl(dsa::Object_Event<T> event)
	{
		if (event.copying() || event.moving())
		{
			auto source_allocation =
			    find_containing_allocation(event.source());
			if (source_allocation == m_allocations.end())
			{
				m_errors.insert(assign_from_uninitialized_memory);
			}
			else
			{
				add_error_if_any(
				    (*source_allocation)->process_source_event(event));
			}
		}

		auto destination_allocation =
		    find_containing_allocation(event.destination());

		if (event.type() == dsa::Object_Event_Type::Before_Construct
		    && destination_allocation == m_allocations.end())
		{
			// If we do not find an allocation with this address we
			// assume that this is a stack variable. This is not
			// correct because we could be constructing on memory
			// which we are not aware of. However, due to the way
			// Memory_Monitor works it is the best we can do
			add_allocation(
			    detail::Allocation_Type::FromConstruct,
			    event.destination(),
			    1);
			destination_allocation = m_allocations.end() - 1;
		}

		add_error_if_any(
		    (*destination_allocation)->process_destination_event(event));
	}

	template<typename T>
	void add_allocation(detail::Allocation_Type type, T *address, size_t count)
	{
		m_allocations.emplace_back(
		    std::make_unique<detail::Allocation_Block_Typed<T>>(
			type,
			address,
			count));
	}

	void add_error_if_any(std::optional<std::string> &&error)
	{
		if (error.has_value())
		{
			m_errors.emplace(std::move(error.value()));
		}
	}

	template<typename T>
	auto before_deallocate_impl(T *address, size_t count) -> bool
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
	void on_deallocate(T *address)
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
};

} // namespace test

#endif
