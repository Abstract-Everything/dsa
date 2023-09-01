#ifndef VISUAL_MEMORY_HPP
#define VISUAL_MEMORY_HPP

#include "dsa/memory_monitor.hpp"
#include "dsa/memory_representation.hpp"

#include <imgui.h>

#include <algorithm>
#include <any>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <functional>
#include <map>
#include <optional>
#include <ostream>
#include <vector>

// TODO: Remove use of numeric_address - use std::any with type casts instead
namespace visual::components
{

enum class Allocation_Type
{
	// If we receive an event for a construction of an element without a prior event for a
	// memory allocation, it is assumed that this memory lives in the stack
	Stack,
	// If we receive an allocate event it means that the code requested a memory block which
	// generally goes on the heap
	Heap,
};

struct VisualAddressBoudingBox
{
	ImVec2 centre;
	ImVec2 size;

	VisualAddressBoudingBox() = default;
	VisualAddressBoudingBox(ImVec2 centre, ImVec2 size);

	auto intersect(ImVec2 from) -> ImVec2;
};

using VisualAddressElementMap = std::map<uintptr_t, VisualAddressBoudingBox>;

class Allocation_Element
{
	using Element = std::unique_ptr<Allocation_Element>;

 public:
	virtual ~Allocation_Element() = default;

	Allocation_Element(Allocation_Element const &)                     = delete;
	auto operator=(Allocation_Element const &) -> Allocation_Element & = delete;
	Allocation_Element(Allocation_Element &&)                          = delete;
	auto operator=(Allocation_Element &&) -> Allocation_Element      & = delete;

	[[nodiscard]] virtual auto pointer() const -> bool        = 0;
	[[nodiscard]] virtual auto address() const -> uintptr_t   = 0;
	[[nodiscard]] virtual auto element_size() const -> size_t = 0;

	[[nodiscard]] auto leaf() const -> bool;
	[[nodiscard]] auto initialised() const -> bool;
	void               move();
	void               assign(std::string value);
	void               construct(std::string value);
	void               destroy();

	template<typename T>
	void start_construction(T *address);

	template<typename T>
	[[nodiscard]] auto field_at(T const *address) const
	    -> std::optional<std::reference_wrapper<Allocation_Element const>> {
		return field_at_impl(address);
	}

	template<typename T>
	[[nodiscard]] auto field_at(T const *address)
	    -> std::optional<std::reference_wrapper<Allocation_Element>> {
		return field_at_impl(address);
	}

	void draw_values(ImDrawList *draw, ImVec2 position, VisualAddressElementMap &address_map);
	void draw_pointers(ImDrawList *draw, VisualAddressElementMap const &address_map);
	[[nodiscard]] auto size() const -> ImVec2;

 protected:
	Allocation_Element() = default;

	[[nodiscard]] virtual auto match_address(std::any const &address) const -> bool = 0;

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

	State                m_state = State::Uninitialised;
	std::string          m_value;
	std::vector<Element> m_fields;

	template<typename T>
	[[nodiscard]] auto field_at_impl(T const *address)
	    -> std::optional<std::reference_wrapper<Allocation_Element>> {
		const uintptr_t raw_address = dsa::numeric_address(address);
		assert(contains_address(raw_address));
		if (match_address(address))
		{
			return *this;
		}

		auto element =
		    std::find_if(m_fields.begin(), m_fields.end(), [&](auto const &element) {
			    return element->contains_address(raw_address);
		    });

		if (element == m_fields.end())
		{
			return std::nullopt;
		}

		return (*element)->field_at_impl(address);
	}

	void update_value(std::string &&value);

	[[nodiscard]] auto contains_address(uintptr_t target) const -> bool;
};

template<typename T>
class Allocation_Element_Typed : public Allocation_Element
{
 public:
	explicit Allocation_Element_Typed(T *address) : Allocation_Element(), m_address(address) {
	}

	[[nodiscard]] auto pointer() const -> bool override {
		return std::is_pointer_v<T>;
	}

	[[nodiscard]] auto address() const -> uintptr_t override {
		return dsa::numeric_address(m_address);
	}

	[[nodiscard]] auto element_size() const -> size_t override {
		return sizeof(*m_address);
	}

	[[nodiscard]] auto match_address(std::any const &address) const -> bool override {
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

 private:
	T *m_address;
};

template<typename T>
void Allocation_Element::start_construction(T *address) {
	assert(contains_address(dsa::numeric_address(address)));
	if (match_address(address))
	{
		m_state = State::Constructing;
		return;
	}

	auto element = std::find_if(
	    m_fields.begin(),
	    m_fields.end(),
	    [&](std::unique_ptr<Allocation_Element> const &element) {
		    return element->contains_address(dsa::numeric_address(address));
	    });

	if (element == m_fields.end())
	{
		auto pointer = std::make_unique<Allocation_Element_Typed<T>>(address);
		m_fields.emplace_back(std::move(pointer));
		element = m_fields.end() - 1;
	}

	(*element)->start_construction(address);
}

class Allocation_Block
{
 public:
	using Element = std::unique_ptr<Allocation_Element>;

	virtual ~Allocation_Block() = default;

	Allocation_Block(Allocation_Block const &)                     = delete;
	auto operator=(Allocation_Block const &) -> Allocation_Block & = delete;
	Allocation_Block(Allocation_Block &&)                          = delete;
	auto operator=(Allocation_Block &&) -> Allocation_Block      & = delete;

	[[nodiscard]] auto count() const -> size_t;
	[[nodiscard]] auto heap() const -> bool;
	[[nodiscard]] auto stack() const -> bool;
	[[nodiscard]] auto match_address(uintptr_t target) const -> bool;
	[[nodiscard]] auto contains(uintptr_t target) const -> bool;
	[[nodiscard]] auto all_elements_destroyed() const -> bool;

	[[nodiscard]] virtual auto address() const -> uintptr_t   = 0;
	[[nodiscard]] auto virtual element_size() const -> size_t = 0;

	template<typename T>
	[[nodiscard]] auto field_at(T const *address) const
	    -> std::optional<std::reference_wrapper<Allocation_Element const>> {
		return element(dsa::numeric_address(address)).field_at(address);
	}

	template<typename T>
	[[nodiscard]] auto field_at(T const *address)
	    -> std::optional<std::reference_wrapper<Allocation_Element>> {
		return element(dsa::numeric_address(address)).field_at(address);
	}

	template<typename T>
	void start_construction(T *address) {
		element(dsa::numeric_address(address)).start_construction(address);
	}

	void draw_values(ImDrawList *draw, ImVec2 position, VisualAddressElementMap &address_map);
	void draw_pointers(ImDrawList *draw, VisualAddressElementMap const &address_map);
	[[nodiscard]] auto size() const -> ImVec2;

 protected:
	std::vector<Element> m_elements;

	explicit Allocation_Block(Allocation_Type allocation_type);

 private:
	Allocation_Type m_allocation_type;

	[[nodiscard]] auto element_index(uintptr_t element_address) const -> size_t;
	[[nodiscard]] auto element(uintptr_t element_address) -> Allocation_Element &;
	[[nodiscard]] auto element(uintptr_t element_address) const -> Allocation_Element const &;
};

template<typename Type>
class Allocation_Block_Typed : public Allocation_Block
{
 public:
	Allocation_Block_Typed(Allocation_Type allocation_type, Type *address, size_t count)
	    : Allocation_Block(allocation_type)
	    , m_address(address) {
		m_elements.reserve(count);
		for (size_t i = 0; i < count; ++i)
		{
			m_elements.emplace_back(
			    std::make_unique<Allocation_Element_Typed<Type>>(address + i));
		}
	}

	[[nodiscard]] auto address() const -> uintptr_t override {
		return dsa::numeric_address(m_address);
	}

	[[nodiscard]] auto element_size() const -> size_t override {
		return sizeof(Type);
	}

 private:
	Type *m_address;
};

class Memory
{
	using Allocation  = std::unique_ptr<Allocation_Block>;
	using Allocations = std::vector<Allocation>;

 public:
	void push(dsa::Memory_Monitor_Event auto &&event) {
		m_events.push_back([=]() { return process_event(event); });
	}

	void draw();

	void update();

 private:
	using Update_Has_Visual_Effect = bool;
	std::deque<std::function<Update_Has_Visual_Effect()>> m_events;
	std::vector<Allocation>                               m_allocations;

	auto draw_allocations(ImDrawList *draw) -> VisualAddressElementMap;
	void draw_nullptr(ImDrawList *draw, ImVec2 position, VisualAddressElementMap &address_map);
	void draw_pointers(ImDrawList *draw, VisualAddressElementMap const &address_map);

	template<typename T>
	Update_Has_Visual_Effect process_event(dsa::Allocation_Event<T> event) {
		switch (event.type())
		{
		case dsa::Allocation_Event_Type::Allocate:
			add_allocation(Allocation_Type::Heap, event.address(), event.count());
			break;

		case dsa::Allocation_Event_Type::Deallocate:
			remove_allocation(event.address());
			break;
		}
		return true;
	}

	template<typename T>
	Update_Has_Visual_Effect process_event(dsa::Object_Event<T> event) {
		auto const destination = field_at(event.destination());
		assert(
		    (destination.has_value()
		     || event.type() == dsa::Object_Event_Type::Before_Construct)
		    && "Field should only be absent when we first construct "
		       "it");

		if (event.moving())
		{
			assert(
			    field_at(event.source()).has_value()
			    && "We expect the source to be monitored");
			field_at(event.source()).value().get().move();
		}

		switch (event.type())
		{
		case dsa::Object_Event_Type::Before_Construct:
			before_construct(event.destination());
			break;

		case dsa::Object_Event_Type::Construct:
		case dsa::Object_Event_Type::Copy_Construct:
		case dsa::Object_Event_Type::Move_Construct:
			destination.value().get().construct(event.destination_value());
			break;

		case dsa::Object_Event_Type::Copy_Assign:
		case dsa::Object_Event_Type::Underlying_Copy_Assign:
		case dsa::Object_Event_Type::Underlying_Move_Assign:
		case dsa::Object_Event_Type::Move_Assign:
			destination.value().get().assign(event.destination_value());
			break;

		case dsa::Object_Event_Type::Destroy:
			destroy_field(event.destination(), destination.value());
			break;
		}

		auto allocation = allocation_containing(dsa::numeric_address(event.destination()));
		return allocation.has_value() && allocation.value().get().heap();
	}

	template<typename T>
	void add_allocation(Allocation_Type type, T *address, size_t count) {
		m_allocations.emplace_back(
		    std::make_unique<Allocation_Block_Typed<T>>(type, address, count));
	}

	template<typename T>
	void remove_allocation(T *address) {
		[[maybe_unused]] size_t elements_erased =
		    std::erase_if(m_allocations, [&](auto const &allocation) {
			    return allocation->match_address(dsa::numeric_address(address));
		    });
		assert(
		    elements_erased != 0
		    && "All events received should have a corresponding allocation");
		assert(
		    elements_erased == 1
		    && "There should never be more than a single allocation with the same address");
	}

	[[nodiscard]] std::optional<std::reference_wrapper<Allocation_Block>> allocation_containing(
	    uintptr_t raw_address);

	template<typename T>
	void before_construct(T *address) {
		if (!allocation_containing(dsa::numeric_address(address)).has_value())
		{
			// If we do not find an allocation with this
			// address we assume that this is a stack
			// variable. This is not correct because we
			// could be constructing on memory which we are
			// not aware of. However, due to the way
			// Memory_Monitor works it is the best we can do
			add_allocation(Allocation_Type::Stack, address, 1);
		}

		for (auto &allocation : m_allocations)
		{
			if (allocation->contains(dsa::numeric_address(address)))
			{
				allocation->start_construction(address);
			}
		}
	}

	template<typename T>
	void destroy_field(T *address, Allocation_Element &field) {
		field.destroy();

		auto allocation_opt = allocation_containing(field.address());
		assert(allocation_opt.has_value() && "TODO");

		Allocation_Block &allocation = allocation_opt.value().get();
		if (!allocation.heap() && allocation.all_elements_destroyed())
		{
			remove_allocation(address);
		}
	}

	template<typename T>
	auto field_at(T const *address) -> std::optional<std::reference_wrapper<Allocation_Element>> {
		for (auto &allocation : m_allocations)
		{
			if (allocation->contains(dsa::numeric_address(address)))
			{
				return allocation->field_at(address);
			}
		}

		return std::nullopt;
	}
};

} // namespace visual::components

#endif
