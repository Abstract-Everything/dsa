#ifndef TEST_DSA_STATIC_ALLOCATION_VERIFIER_HPP
#define TEST_DSA_STATIC_ALLOCATION_VERIFIER_HPP

#include <dsa/allocator_traits.hpp>
#include <dsa/default_allocator.hpp>
#include <dsa/memory_monitor.hpp>
#include <dsa/memory_representation.hpp>

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

class Alloc_Traits_Misuse : public std::runtime_error
{
 public:
	using std::runtime_error::runtime_error;
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
			if (allocation->owns_allocation() && !allocation->all_elements_destroyed())
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
		instance()->memory_representation.process_event(event);
	}

	template<typename T>
	static void process_object_event(dsa::Object_Event<T> event)
	{
		instance()->process_object_event_impl(event);
	}

 private:
	dsa::Memory_Representation memory_representation;
	std::set<std::string>      m_errors;

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

		memory_representation.process_event(event);
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

		if (allocation.owns_allocation() && !allocation.all_elements_destroyed())
		{
			allocation.cleanup();
			add_error(object_leaked);
		}

		return true;
	}
};

} // namespace test

#endif
