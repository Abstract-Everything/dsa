#ifndef DSA_MEMORY_REPRESENTATION_HPP
#define DSA_MEMORY_REPRESENTATION_HPP

#include <dsa/memory_monitor.hpp>

#include <algorithm>
#include <any>
#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

namespace dsa
{

template<typename T>
auto numeric_address(T *pointer) -> uintptr_t {
	return reinterpret_cast<uintptr_t>(pointer);
}

class Allocation_Element
{
	using Element = std::unique_ptr<Allocation_Element>;

 public:
	virtual ~Allocation_Element() = default;

	[[nodiscard]] virtual auto clone() const -> std::unique_ptr<Allocation_Element> = 0;

	[[nodiscard]] virtual auto pointer() const -> bool = 0;

	[[nodiscard]] virtual auto address() const -> uintptr_t = 0;

	[[nodiscard]] auto leaf() const -> bool {
		return m_fields.empty();
	}

	[[nodiscard]] auto initialised() const -> bool {
		return m_state == State::Initialised;
	}

	[[nodiscard]] auto assignable() const -> bool {
		return m_state == State::Initialised || m_state == State::Moved;
	}

	[[nodiscard]] auto destructable() const -> bool {
		return m_state == State::Initialised || m_state == State::Moved;
	}

	void move() {
		m_state = State::Moved;
	}

	void assign(std::string value) {
		m_state = State::Initialised;
		update_value(std::move(value));
	}

	void construct(std::string value) {
		m_state = State::Initialised;
		update_value(std::move(value));
	}

	void destroy() {
		m_state = State::Uninitialised;
	}

	[[nodiscard]] auto value() const -> std::string {
		return initialised() ? m_value : "";
	}

	template<typename T>
	void start_construction(T *address);

	[[nodiscard]] std::vector<Element> const &fields() const {
		return m_fields;
	}

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

 protected:
	Allocation_Element() = default;

	Allocation_Element(Allocation_Element const &element)
	    : m_state(element.m_state)
	    , m_value(element.m_value) {
		m_fields.reserve(element.m_fields.size());
		for (auto const &field : element.m_fields)
		{
			m_fields.emplace_back(field->clone());
		}
	}

	[[nodiscard]] virtual auto match_address(std::any const &address) const -> bool = 0;

	[[nodiscard]] virtual auto contains_address(uintptr_t address) const -> bool = 0;

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
		const uintptr_t raw_address = numeric_address(address);
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

	void update_value(std::string &&value) {
		m_value = std::move(value);
	}
};

template<typename T>
class Allocation_Element_Typed : public Allocation_Element
{
 public:
	explicit Allocation_Element_Typed(T *address) : Allocation_Element(), m_address(address) {
	}

	Allocation_Element_Typed(Allocation_Element_Typed const &element)
	    : Allocation_Element(element)
	    , m_address(element.m_address) {
	}

	[[nodiscard]] auto clone() const -> std::unique_ptr<Allocation_Element> override {
		return std::make_unique<Allocation_Element_Typed>(*this);
	}

	[[nodiscard]] auto pointer() const -> bool override {
		return std::is_pointer_v<T>;
	}

	[[nodiscard]] auto address() const -> uintptr_t override {
		return numeric_address(m_address);
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

	[[nodiscard]] auto contains_address(uintptr_t address) const -> bool override {
		uintptr_t base = numeric_address(m_address);
		return address >= base && address < base + sizeof(*m_address);
	}

 private:
	T *m_address;
};

template<typename T>
void Allocation_Element::start_construction(T *address) {
	assert(contains_address(numeric_address(address)));
	if (match_address(address))
	{
		m_state = State::Constructing;
		return;
	}

	auto element = std::find_if(
	    m_fields.begin(),
	    m_fields.end(),
	    [&](std::unique_ptr<Allocation_Element> const &element) {
		    return element->contains_address(numeric_address(address));
	    });
	if (element == m_fields.end())
	{
		auto pointer = std::make_unique<Allocation_Element_Typed<T>>(address);
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
	using Element = std::unique_ptr<Allocation_Element>;

	Allocation_Block()          = default;
	virtual ~Allocation_Block() = default;

	[[nodiscard]] virtual auto clone() const -> std::unique_ptr<Allocation_Block> = 0;

	[[nodiscard]] virtual auto count() const -> size_t = 0;

	[[nodiscard]] virtual auto address() const -> uintptr_t = 0;

	[[nodiscard]] virtual auto match_address(uintptr_t address) const -> bool = 0;

	[[nodiscard]] virtual auto contains(uintptr_t address) const -> bool = 0;

	[[nodiscard]] virtual auto owns_allocation() const -> bool = 0;

	[[nodiscard]] virtual auto all_elements_destroyed() const -> bool = 0;

	[[nodiscard]] std::vector<Element> const &fields() const {
		return m_elements;
	}

	template<typename T>
	[[nodiscard]] auto field_at(T const *address) const
	    -> std::optional<std::reference_wrapper<Allocation_Element const>> {
		return element(numeric_address(address)).field_at(address);
	}

	template<typename T>
	[[nodiscard]] auto field_at(T const *address)
	    -> std::optional<std::reference_wrapper<Allocation_Element>> {
		return element(numeric_address(address)).field_at(address);
	}

	template<typename T>
	void start_construction(T *address) {
		element(numeric_address(address)).start_construction(address);
	}

	virtual void cleanup()    = 0;
	virtual void deallocate() = 0;

 protected:
	std::vector<Element> m_elements;

 private:
	[[nodiscard]] virtual auto element(uintptr_t address) -> Allocation_Element & = 0;

	[[nodiscard]] virtual auto element(uintptr_t address) const -> Allocation_Element const & = 0;
};

template<typename Type>
class Allocation_Block_Typed : public Allocation_Block
{
 public:
	Allocation_Block_Typed(Allocation_Type allocation_type, Type *address, size_t count)
	    : Allocation_Block()
	    , m_allocation_type(allocation_type)
	    , m_address(address) {
		m_elements.reserve(count);
		for (size_t i = 0; i < count; ++i)
		{
			m_elements.emplace_back(
			    std::make_unique<Allocation_Element_Typed<Type>>(address + i));
		}
	}

	Allocation_Block_Typed(Allocation_Block_Typed const &block)
	    : Allocation_Block()
	    , m_allocation_type(block.m_allocation_type)
	    , m_address(block.m_address) {
		m_elements.reserve(block.m_elements.size());
		for (auto const &element : block.m_elements)
		{
			m_elements.emplace_back(element->clone());
		}
	}

	Allocation_Block_Typed(Allocation_Block_Typed &&block) noexcept = default;

	~Allocation_Block_Typed() override = default;

	[[nodiscard]] auto clone() const -> std::unique_ptr<Allocation_Block> override {
		auto copy = std::make_unique<Allocation_Block_Typed>(*this);
		return std::move(copy);
	}

	[[nodiscard]] auto count() const -> size_t override {
		return m_elements.size();
	}

	[[nodiscard]] auto address() const -> uintptr_t override {
		return numeric_address(m_address);
	}

	[[nodiscard]] auto match_address(uintptr_t target) const -> bool override {
		return address() == target;
	}

	[[nodiscard]] auto contains(uintptr_t target) const -> bool override {
		return target >= numeric_address(m_address)
		       && target < numeric_address(m_address + count());
	}

	[[nodiscard]] auto owns_allocation() const -> bool override {
		return m_allocation_type == Allocation_Type::Owned;
	}

	[[nodiscard]] auto all_elements_destroyed() const -> bool override {
		return std::none_of(m_elements.begin(), m_elements.end(), [](Element const &element) {
			return element->initialised();
		});
	}

	void cleanup() override {
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

	void deallocate() override {
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

	Allocation_Type m_allocation_type;
	Type           *m_address;

	[[nodiscard]] auto element_index(uintptr_t address) const -> size_t {
		assert(contains(address) && "Expected the address to be inside of the allocation");

		auto base_address     = numeric_address(m_address);
		auto relative_address = address - base_address;
		return relative_address / sizeof(Type);
	}

	[[nodiscard]] auto element(uintptr_t address) -> Allocation_Element & override {
		return *m_elements[element_index(address)];
	}

	[[nodiscard]] auto element(uintptr_t address) const -> Allocation_Element const & override {
		return *m_elements[element_index(address)];
	}
};

class Memory_Representation
{
	using Allocation  = std::unique_ptr<Allocation_Block>;
	using Allocations = std::vector<Allocation>;

 public:
	Memory_Representation() = default;

	Memory_Representation(Memory_Representation const &memory_representation) {
		m_allocations.reserve(memory_representation.m_allocations.size());
		for (auto const &allocation : memory_representation.m_allocations)
		{
			m_allocations.emplace_back(allocation->clone());
		}
	}

	Memory_Representation(Memory_Representation &&memory_representation) = default;

 public:
	[[nodiscard]] Allocations const &allocations() const {
		return m_allocations;
	}

	template<typename T>
	[[nodiscard]] std::optional<std::reference_wrapper<Allocation_Block>> allocation_at(T *address) {
		for (auto &allocation : m_allocations)
		{
			if (allocation->match_address(numeric_address(address)))
			{
				return *allocation;
			}
		}
		return std::nullopt;
	}

	template<typename T>
	auto field_at(T const *address) const
	    -> std::optional<std::reference_wrapper<Allocation_Element>> {
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
	auto field_at(T const *address) -> std::optional<std::reference_wrapper<Allocation_Element>> {
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
	void process_event(dsa::Allocation_Event<T> event) {
		switch (event.type())
		{
		case dsa::Allocation_Event_Type::Allocate:
			add_allocation(Allocation_Type::Owned, event.address(), event.count());
			break;

		case dsa::Allocation_Event_Type::Deallocate:
			std::erase_if(m_allocations, [&](auto const &allocation) {
				return allocation->match_address(numeric_address(event.address()));
			});
			break;
		}
	}

	template<typename T>
	void process_event(dsa::Object_Event<T> event) {
		if (event.moving())
		{
			auto result = field_at(event.source());
			assert(result.has_value() && "We expect the source to be monitored");
			result.value().get().move();
		}

		auto const field = field_at(event.destination());
		assert(
		    (field.has_value()
		     || event.type() == dsa::Object_Event_Type::Before_Construct)
		    && "Field should only be absent when we first construct "
		       "it");

		switch (event.type())
		{
		case dsa::Object_Event_Type::Before_Construct:
		{
			if (!allocation_containing(numeric_address(event.destination())).has_value())
			{
				// If we do not find an allocation with this
				// address we assume that this is a stack
				// variable. This is not correct because we
				// could be constructing on memory which we are
				// not aware of. However, due to the way
				// Memory_Monitor works it is the best we can do
				add_allocation(Allocation_Type::FromConstruct, event.destination(), 1);
			}

			for (auto &allocation : m_allocations)
			{
				if (allocation->contains(numeric_address(event.destination())))
				{
					allocation->start_construction(event.destination());
				}
			}
		}
		break;

		case dsa::Object_Event_Type::Construct:
		case dsa::Object_Event_Type::Copy_Construct:
		case dsa::Object_Event_Type::Move_Construct:
			field.value().get().construct(event.destination_value());
			break;

		case dsa::Object_Event_Type::Copy_Assign:
		case dsa::Object_Event_Type::Underlying_Copy_Assign:
		case dsa::Object_Event_Type::Underlying_Move_Assign:
		case dsa::Object_Event_Type::Move_Assign:
			field.value().get().assign(event.destination_value());
			break;

		case dsa::Object_Event_Type::Destroy:
			field.value().get().destroy();
			{
				auto allocation =
				    allocation_containing(field.value().get().address());
				assert(allocation.has_value());
				Allocation_Block &a = allocation.value().get();
				if (!a.owns_allocation() && a.all_elements_destroyed())
				{
					uintptr_t address = a.address();
					m_allocations.erase(
					    std::remove_if(
						m_allocations.begin(),
						m_allocations.end(),
						[&](Allocation const &block) {
							return block->match_address(address);
						}),
					    m_allocations.end());
				}
			}
			break;
		}
	}

	void free_heap_allocations() {
		for (auto &allocation : m_allocations)
		{
			allocation->cleanup();
			allocation->deallocate();
		}

		std::erase_if(m_allocations, [](auto const &allocation) {
			return allocation->owns_allocation();
		});
	}

 private:
	Allocations m_allocations;

	template<typename T>
	void add_allocation(Allocation_Type type, T *address, size_t count) {
		m_allocations.emplace_back(
		    std::make_unique<Allocation_Block_Typed<T>>(type, address, count));
	}

	[[nodiscard]] std::optional<std::reference_wrapper<Allocation_Block>> allocation_containing(
	    uintptr_t raw_address) {
		for (auto &allocation : m_allocations)
		{
			if (allocation->contains(raw_address))
			{
				return *allocation;
			}
		}
		return std::nullopt;
	}
};

} // namespace dsa

#endif
