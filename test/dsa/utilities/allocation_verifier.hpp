#ifndef TEST_DSA_STATIC_ALLOCATION_VERIFIER_HPP
#define TEST_DSA_STATIC_ALLOCATION_VERIFIER_HPP

#include <dsa/allocator_traits.hpp>
#include <dsa/default_allocator.hpp>
#include <dsa/memory_monitor.hpp>

#include <algorithm>
#include <any>
#include <cassert>
#include <concepts>
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

	[[nodiscard]] auto assignable() const -> bool
	{
		return m_state == State::Initialised || m_state == State::Moved;
	}

	[[nodiscard]] auto destructable() const -> bool
	{
		return m_state == State::Initialised || m_state == State::Moved;
	}

	auto move()
	{
		m_state = State::Moved;
	}

	auto assign()
	{
		m_state = State::Initialised;
	}

	auto construct()
	{
		m_state = State::Initialised;
	}

	auto destroy()
	{
		m_state = State::Uninitialised;
	}

	template<typename T>
	void start_construction(T *address);

	template<typename T>
	[[nodiscard]] auto field_at(T const *address) const
	    -> std::optional<std::reference_wrapper<const Allocation_Element>>
	{
		return field_at_impl(address);
	}

	template<typename T>
	[[nodiscard]] auto field_at(T const *address)
	    -> std::optional<std::reference_wrapper<Allocation_Element>>
	{
		return field_at_impl(address);
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
	[[nodiscard]] auto field_at_impl(T const *address)
	    -> std::optional<std::reference_wrapper<Allocation_Element>>
	{
		const uintptr_t raw_address = numeric_address(address);
		assert(contains_address(raw_address));
		if (match_address(address))
		{
			return *this;
		}

		auto element = std::find_if(
		    m_fields.begin(),
		    m_fields.end(),
		    [&](auto const &element)
		    { return element->contains_address(raw_address); });

		if (element == m_fields.end())
		{
			return std::nullopt;
		}

		return (*element)->field_at_impl(address);
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
void Allocation_Element::start_construction(T *address)
{
	assert(contains_address(numeric_address(address)));
	if (match_address(address))
	{
		m_state = State::Constructing;
		return;
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

	[[nodiscard]] virtual auto all_heap_elements_destroyed() const
	    -> bool = 0;

	template<typename T>
	[[nodiscard]] auto field_at(const T *address) const
	    -> std::optional<std::reference_wrapper<const Allocation_Element>>
	{
		return element(numeric_address(address)).field_at(address);
	}

	template<typename T>
	[[nodiscard]] auto field_at(const T *address)
	    -> std::optional<std::reference_wrapper<Allocation_Element>>
	{
		return element(numeric_address(address)).field_at(address);
	}

	template<typename T>
	void start_construction(T *address)
	{
		element(numeric_address(address)).start_construction(address);
	}

	virtual void cleanup()    = 0;
	virtual void deallocate() = 0;

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

	[[nodiscard]] auto all_heap_elements_destroyed() const -> bool override
	{
		return !owns_allocation()
		       || std::none_of(
			   m_elements.begin(),
			   m_elements.end(),
			   [](Element const &element)
			   { return element->initialised(); });
	}

	void cleanup() override
	{
		if (!owns_allocation())
		{
			return;
		}

		Allocator allocator;
		for (size_t i = 0; i < count(); ++i)
		{
			if (m_elements[i]->initialised())
			{
				Alloc_Traits::destroy(allocator, m_address + i);
			}
		}
	}

	void deallocate() override
	{
		if (!owns_allocation())
		{
			return;
		}

		Allocator allocator;
		Alloc_Traits::deallocate(allocator, m_address, count());
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

class Memory_Representation
{
	using Allocation  = std::unique_ptr<detail::Allocation_Block>;
	using Allocations = std::vector<Allocation>;

 public:
	[[nodiscard]] Allocations const &allocations() const
	{
		return m_allocations;
	}

	template<typename T>
	[[nodiscard]] std::optional<std::reference_wrapper<detail::Allocation_Block>> allocation_at(
	    T *address)
	{
		uintptr_t raw_address = detail::numeric_address(address);
		for (auto &allocation : m_allocations)
		{
			if (allocation->match_address(raw_address))
			{
				return *allocation;
			}
		}
		return std::nullopt;
	}

	template<typename T>
	auto field_at(const T *address) const
	    -> std::optional<std::reference_wrapper<Allocation_Element>>
	{
		for (auto const &allocation : m_allocations)
		{
			if (allocation->contains(numeric_address(address)))
			{
				return allocation->field_at(address);
			}
		}

		return std::nullopt;
	}

	template<typename T>
	auto field_at(const T *address)
	    -> std::optional<std::reference_wrapper<Allocation_Element>>
	{
		for (auto &allocation : m_allocations)
		{
			if (allocation->contains(numeric_address(address)))
			{
				return allocation->field_at(address);
			}
		}

		return std::nullopt;
	}

	template<typename T>
	void process_allocation_event(dsa::Allocation_Event<T> event)
	{
		switch (event.type())
		{
		case dsa::Allocation_Event_Type::Allocate:
			add_allocation(
			    detail::Allocation_Type::Owned,
			    event.address(),
			    event.count());
			break;

		case dsa::Allocation_Event_Type::Deallocate:
			std::erase_if(
			    m_allocations,
			    [&](auto const &allocation)
			    {
				    return allocation->match_address(
					detail::numeric_address(event.address()));
			    });
			break;
		}
	}

	template<typename T>
	void process_object_event(dsa::Object_Event<T> event)
	{
		if (event.moving())
		{
			auto result = field_at(event.source());
			assert(result.has_value() && "We expect the source to be monitored");
			result.value().get().move();
		}

		const auto field = field_at(event.destination());
		assert(
		    (field.has_value()
		     || event.type() == dsa::Object_Event_Type::Before_Construct)
		    && "Field should only be absent when we first construct it");

		switch (event.type())
		{
		case dsa::Object_Event_Type::Before_Construct:
		{
			if (!field_at(event.destination()).has_value())
			{
				// If we do not find an allocation with this
				// address we assume that this is a stack
				// variable. This is not correct because we
				// could be constructing on memory which we are
				// not aware of. However, due to the way
				// Memory_Monitor works it is the best we can do
				add_allocation(
				    detail::Allocation_Type::FromConstruct,
				    event.destination(),
				    1);
			}

			for (auto &allocation : m_allocations)
			{
				if (allocation->contains(
					numeric_address(event.destination())))
				{
					allocation->start_construction(
					    event.destination());
				}
			}
		}
		break;

		case dsa::Object_Event_Type::Construct:
		case dsa::Object_Event_Type::Copy_Construct:
		case dsa::Object_Event_Type::Move_Construct:
			field.value().get().construct();
			break;

		case dsa::Object_Event_Type::Copy_Assign:
		case dsa::Object_Event_Type::Underlying_Copy_Assign:
		case dsa::Object_Event_Type::Underlying_Move_Assign:
		case dsa::Object_Event_Type::Move_Assign:
			field.value().get().assign();
			break;

		case dsa::Object_Event_Type::Destroy:
			field.value().get().destroy();
			break;
		}
	}

	void free_heap_allocations()
	{
		for (auto &allocation : m_allocations)
		{
			allocation->cleanup();
			allocation->deallocate();
		}

		std::erase_if(
		    m_allocations,
		    [](auto const &allocation)
		    { return allocation->owns_allocation(); });
	}

 private:
	Allocations m_allocations;

	template<typename T>
	void add_allocation(detail::Allocation_Type type, T *address, size_t count)
	{
		m_allocations.emplace_back(
		    std::make_unique<detail::Allocation_Block_Typed<T>>(
			type,
			address,
			count));
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
		for (auto const &allocation : memory_representation.allocations())
		{
			if (!allocation->all_heap_elements_destroyed())
			{
				add_error(object_leaked);
			}

			if (allocation->owns_allocation())
			{
				add_error(memory_leaked);
			}
		}

		memory_representation.free_heap_allocations();

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
		instance()->memory_representation.process_allocation_event(event);
	}

	template<typename T>
	static void process_object_event(dsa::Object_Event<T> event)
	{
		instance()->process_object_event_impl(event);
	}

 private:
	detail::Memory_Representation memory_representation;
	std::set<std::string>         m_errors;

	template<typename T>
	void process_object_event_impl(dsa::Object_Event<T> event)
	{
		if (event.copying() || event.moving())
		{
			const auto source_field =
			    memory_representation.field_at(event.source());

			if (!source_field.has_value()
			    || !source_field.value().get().initialised())
			{
				add_error(assign_from_uninitialized_memory);
				return;
			}
		}

		const auto destination_field =
		    memory_representation.field_at(event.destination());

		switch (event.type())
		{
		case dsa::Object_Event_Type::Before_Construct:
			if (destination_field.has_value()
			    && destination_field.value().get().initialised())
			{
				add_error(object_leaked);
				return;
			}
			break;

		case dsa::Object_Event_Type::Construct:
		case dsa::Object_Event_Type::Copy_Construct:
		case dsa::Object_Event_Type::Move_Construct:
			break;

		case dsa::Object_Event_Type::Copy_Assign:
		case dsa::Object_Event_Type::Underlying_Copy_Assign:
		case dsa::Object_Event_Type::Move_Assign:
		case dsa::Object_Event_Type::Underlying_Move_Assign:
			if (!(destination_field.has_value()
			      && destination_field.value().get().assignable()))
			{
				add_error(assign_uninitialized_memory);
				return;
			}
			break;

		case dsa::Object_Event_Type::Destroy:
			if (!(destination_field.has_value()
			      && destination_field.value().get().destructable()))
			{
				add_error(destroying_nonconstructed_memory);
				return;
			}
			break;
		}

		memory_representation.process_object_event(event);
	}

	void add_error(std::string &&error)
	{
		m_errors.emplace(std::move(error));
	}

	template<typename T>
	auto before_deallocate_impl(T *address, size_t count) -> bool
	{
		auto result = memory_representation.allocation_at(address);
		if (!result.has_value())
		{
			add_error(deallocating_unallocated_memory);
			return false;
		}

		auto &allocation = result.value().get();
		if (allocation.count() != count)
		{
			allocation.cleanup();
			add_error(deallocating_count_mismatch);
			return false;
		}

		if (!allocation.all_heap_elements_destroyed())
		{
			allocation.cleanup();
			add_error(object_leaked);
		}

		return true;
	}
};

} // namespace test

#endif
