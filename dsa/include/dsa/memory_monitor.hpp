#ifndef DSA_MEMORY_MONITOR_HPP
#define DSA_MEMORY_MONITOR_HPP

#include <dsa/allocator_traits.hpp>
#include <dsa/default_allocator.hpp>
#include <dsa/type_traits.hpp>

#include <cassert>
#include <cstddef>
#include <memory>
#include <sstream>

namespace dsa
{

namespace detail
{

/// @brief Allows the Element_Monitor to send events before construction starts
class Pre_Construct
{
 public:
	explicit Pre_Construct(std::invocable auto callback) {
		callback();
	}
};

/// @brief Allows the Element_Monitor to act as another type. We either inherit
/// from an object or contain a trivial type
template<typename Base, bool = std::is_class_v<Base>>
class Element_Monitor_Base;

template<typename Base>
class Element_Monitor_Base<Base, true> : public Base
{
 public:
	using Base::Base;

	explicit Element_Monitor_Base(Base const &base) : Base(base) {
	}

	explicit Element_Monitor_Base(Base &&base) : Base(std::move(base)) {
	}

	auto base() -> Base & {
		return *this;
	}

	auto base() const -> Base const & {
		return *this;
	}
};

template<typename Base>
class Element_Monitor_Base<Base, false>
{
 public:
	Element_Monitor_Base() = default;

	explicit Element_Monitor_Base(Base base) : m_base(base) {
	}

	operator Base const &() const {
		return m_base;
	}

	auto base() -> Base & {
		return m_base;
	}

	auto base() const -> Base const & {
		return m_base;
	}

 private:
	Base m_base;
};

} // namespace detail

enum class Allocation_Event_Type
{
	Allocate,
	Deallocate,
};

inline auto operator<<(std::ostream &stream, Allocation_Event_Type type)
    -> std::ostream & {
	switch (type)
	{
	case Allocation_Event_Type::Allocate:
		stream << "Allocation";
		break;

	case Allocation_Event_Type::Deallocate:
		stream << "Deallocation";
		break;
	};
	return stream;
}

enum class Object_Event_Type
{
	Before_Construct,
	Construct,
	Copy_Construct,
	Copy_Assign,
	Underlying_Copy_Assign,
	Move_Construct,
	Move_Assign,
	Underlying_Move_Assign,
	Destroy,
};

inline auto operator<<(std::ostream &stream, Object_Event_Type type)
    -> std::ostream & {
	switch (type)
	{
	case Object_Event_Type::Before_Construct:
		stream << "Before construction";
		break;

	case Object_Event_Type::Construct:
		stream << "Construction";
		break;

	case Object_Event_Type::Destroy:
		stream << "Destruction";
		break;

	case Object_Event_Type::Copy_Construct:
		stream << "Copy construction";
		break;

	case Object_Event_Type::Copy_Assign:
		stream << "Copy assignment";
		break;

	case Object_Event_Type::Underlying_Copy_Assign:
		stream << "Underlying copy assignment";
		break;

	case Object_Event_Type::Move_Construct:
		stream << "Move construction";
		break;

	case Object_Event_Type::Move_Assign:
		stream << "Move assignment";
		break;

	case Object_Event_Type::Underlying_Move_Assign:
		stream << "Underlying move assignment";
		break;
	};
	return stream;
}

/// Describes the parameters passed to an allocate or deallocate call
template<typename T>
class Allocation_Event
{
 public:
	using Type       = T;
	using Event_Type = Allocation_Event_Type;

	Allocation_Event(Allocation_Event_Type type, Type *pointer, size_t count)
	    : m_type(type)
	    , m_pointer(pointer)
	    , m_count(count) {
	}

	auto operator==(Allocation_Event const &event) const -> bool = default;

	friend auto operator<<(std::ostream &stream, Allocation_Event const &event)
	    -> std::ostream & {
		stream << event.m_type << " at address " << event.m_pointer
		       << " with count " << event.m_count;
		return stream;
	}

	[[nodiscard]] auto type() const -> Allocation_Event_Type {
		return m_type;
	}

	auto address() -> Type * {
		return m_pointer;
	}

	auto count() -> size_t {
		return m_count;
	}

 private:
	Allocation_Event_Type m_type;
	Type                 *m_pointer;
	size_t                m_count;
};

/// Describes the parameters passed to functions which modify an object or its
/// lifetime.
template<typename T>
class Object_Event
{
 public:
	using Type       = T;
	using Event_Type = Object_Event_Type;

	Object_Event(Object_Event_Type type, Type *destination)
	    : Object_Event(type, destination, nullptr) {
	}

	Object_Event(Object_Event_Type type, Type *destination, Type const *source)
	    : m_type(type)
	    , m_destination(destination)
	    , m_source(source) {
		switch (type)
		{
		case dsa::Object_Event_Type::Before_Construct:
		case dsa::Object_Event_Type::Construct:
		case dsa::Object_Event_Type::Underlying_Copy_Assign:
		case dsa::Object_Event_Type::Underlying_Move_Assign:
		case dsa::Object_Event_Type::Destroy:
			assert(
			    m_source == nullptr
			    && "We expect the source to be absent for these events");
			break;

		case dsa::Object_Event_Type::Copy_Construct:
		case dsa::Object_Event_Type::Move_Construct:
		case dsa::Object_Event_Type::Copy_Assign:
		case dsa::Object_Event_Type::Move_Assign:
			assert(
			    m_source != nullptr
			    && "We expect the source to be provided for these events");
			break;
		}

		std::stringstream stream;
		stream << *m_destination;
		m_destination_value = stream.str();
	}

	auto operator==(Object_Event const &event) const -> bool = default;

	friend auto operator<<(std::ostream &stream, Object_Event const &event)
	    -> std::ostream & {
		stream << event.m_type << " at address " << event.m_destination;

		if (event.m_source != nullptr)
		{
			switch (event.m_type)
			{
			case Object_Event_Type::Before_Construct:
			case Object_Event_Type::Construct:
			case Object_Event_Type::Destroy:
			case Object_Event_Type::Underlying_Copy_Assign:
			case Object_Event_Type::Underlying_Move_Assign:
				break;

			case Object_Event_Type::Copy_Construct:
			case Object_Event_Type::Copy_Assign:
			case Object_Event_Type::Move_Construct:
			case Object_Event_Type::Move_Assign:
				stream << " from address " << event.m_source;
				break;
			};
		}

		return stream;
	}

	[[nodiscard]] auto type() const -> Object_Event_Type {
		return m_type;
	}

	auto destination() -> Type * {
		return m_destination;
	}

	auto source() -> Type const * {
		return m_source;
	}

	[[nodiscard]] auto destination_value() const -> std::string const & {
		return m_destination_value;
	}

	[[nodiscard]] auto constructing() const -> bool {
		switch (m_type)
		{
		case Object_Event_Type::Construct:
		case Object_Event_Type::Copy_Construct:
		case Object_Event_Type::Move_Construct:
			return true;

		case Object_Event_Type::Before_Construct:
		case Object_Event_Type::Copy_Assign:
		case Object_Event_Type::Underlying_Copy_Assign:
		case Object_Event_Type::Move_Assign:
		case Object_Event_Type::Underlying_Move_Assign:
		case Object_Event_Type::Destroy:
			return false;
		};
	}

	[[nodiscard]] auto copying() const -> bool {
		switch (m_type)
		{
		case Object_Event_Type::Copy_Construct:
		case Object_Event_Type::Copy_Assign:
			return true;

		case Object_Event_Type::Before_Construct:
		case Object_Event_Type::Construct:
		case Object_Event_Type::Underlying_Copy_Assign:
		case Object_Event_Type::Move_Construct:
		case Object_Event_Type::Move_Assign:
		case Object_Event_Type::Underlying_Move_Assign:
		case Object_Event_Type::Destroy:
			return false;
		};
	}

	[[nodiscard]] auto moving() const -> bool {
		switch (m_type)
		{
		case Object_Event_Type::Move_Construct:
		case Object_Event_Type::Move_Assign:
			return true;

		case Object_Event_Type::Before_Construct:
		case Object_Event_Type::Construct:
		case Object_Event_Type::Copy_Construct:
		case Object_Event_Type::Copy_Assign:
		case Object_Event_Type::Underlying_Copy_Assign:
		case Object_Event_Type::Underlying_Move_Assign:
		case Object_Event_Type::Destroy:
			return false;
		};
	}

 private:
	Object_Event_Type m_type;
	Type             *m_destination;
	T const          *m_source;
	std::string       m_destination_value;
};

template<typename Event>
concept Memory_Monitor_Event =
    std::is_same_v<Event, Allocation_Event<typename Event::Type>>
    || std::is_same_v<Event, Object_Event<typename Event::Type>>;

template<typename Handler, typename Type>
concept Memory_Monitor_Event_Handler = requires(
    Allocation_Event<Type> allocation_event,
    Object_Event<Type>     object_event) {
	{
		Handler::before_deallocate(std::move(allocation_event))
	} -> std::same_as<bool>;

	{ Handler::process_allocation_event(std::move(allocation_event)) };
	{ Handler::process_object_event(std::move(object_event)) };
};

/// @brief Wraps around a value in order to monitor its lifetime
template<typename Base, Memory_Monitor_Event_Handler<Base> Handler>
class Element_Monitor
    : private detail::Pre_Construct
    , public detail::Element_Monitor_Base<Base>
{
	using Base_Wrapper = detail::Element_Monitor_Base<Base>;

 public:
	template<typename... Arguments>
	Element_Monitor(Arguments &&...arguments)
	requires(std::is_constructible_v<Base, Arguments...>
		 && !Is_Same_v<Element_Monitor, Arguments...>)
	    : Pre_Construct([this]() { before_construct(); })
	    , Base_Wrapper(std::forward<Arguments>(arguments)...) {
		Handler::process_object_event(Object_Event(
		    Object_Event_Type::Construct,
		    &Base_Wrapper::base()));
	}

	~Element_Monitor() {
		Handler::process_object_event(Object_Event(
		    Object_Event_Type::Destroy,
		    &Base_Wrapper::base()));
	}

	Element_Monitor(Element_Monitor const &element)
	requires std::is_copy_constructible_v<Base>
	    : Pre_Construct([this]() { before_construct(); })
	    , Base_Wrapper(element) {
		Handler::process_object_event(Object_Event(
		    Object_Event_Type::Copy_Construct,
		    &Base_Wrapper::base(),
		    &element.base()));
	}

	auto operator=(Element_Monitor const &element) -> Element_Monitor &
	requires std::is_copy_assignable_v<Base>
	{
		Base_Wrapper::base() = element.base();
		Handler::process_object_event(Object_Event(
		    Object_Event_Type::Copy_Assign,
		    &Base_Wrapper::base(),
		    &element.base()));
		return *this;
	}

	auto operator=(Base const &value) noexcept -> Element_Monitor &
	requires std::is_copy_assignable_v<Base>
	{
		Base_Wrapper::base() = value;
		Handler::process_object_event(Object_Event(
		    Object_Event_Type::Underlying_Copy_Assign,
		    &Base_Wrapper::base()));
		return *this;
	}

	Element_Monitor(Element_Monitor &&element) noexcept
	requires std::is_move_constructible_v<Base>
	    : Pre_Construct([this]() { before_construct(); })
	    , Base_Wrapper(std::move(element)) {
		Handler::process_object_event(Object_Event(
		    Object_Event_Type::Move_Construct,
		    &Base_Wrapper::base(),
		    &element.base()));
	}

	auto operator=(Element_Monitor &&element) noexcept -> Element_Monitor &
	requires std::is_move_assignable_v<Base>
	{
		Base_Wrapper::base() = std::move(element.base());
		Handler::process_object_event(Object_Event(
		    Object_Event_Type::Move_Assign,
		    &Base_Wrapper::base(),
		    &element.base()));
		return *this;
	}

	auto operator=(Base &&value) noexcept -> Element_Monitor &
	requires std::is_move_assignable_v<Base>
	{
		Base_Wrapper::base() = value;
		Handler::process_object_event(Object_Event(
		    Object_Event_Type::Underlying_Move_Assign,
		    &Base_Wrapper::base()));
		return *this;
	}

	friend auto operator<<(std::ostream &stream, Element_Monitor const &monitor)
	    -> std::ostream & {
		return stream << monitor.base();
	}

 private:
	void before_construct() {
		Handler::process_object_event(Object_Event(
		    Object_Event_Type::Before_Construct,
		    &Base_Wrapper::base()));
	}
};

/// @brief Wraps around a value in order to monitor its lifetime
template<bool IsConst, typename Base, Memory_Monitor_Event_Handler<Base> Handler>
class Element_Monitor_Pointer : private detail::Pre_Construct
{
	friend class Element_Monitor_Pointer<!IsConst, Base, Handler>;

	// clang-format off
	using Value               = Element_Monitor<Base, Handler>;
	using Base_Pointer        = Base *;
	using Reference           = Value &;
	using Const_Reference     = Value const &;
	using Interface_Reference = std::conditional_t<IsConst, Const_Reference, Reference>;
	using Pointer             = Value *;
	using Const_Pointer       = Value const *;
	using Interface_Pointer   = std::conditional_t<IsConst, Const_Pointer, Pointer>;
	// clang-format on

 public:
	using iterator_category = std::random_access_iterator_tag;
	using difference_type   = std::ptrdiff_t;
	using value_type        = Value;
	using reference         = Value &;
	using pointer           = Element_Monitor_Pointer;

	Element_Monitor_Pointer(std::nullptr_t = nullptr)
	    : Element_Monitor_Pointer(Pointer(nullptr)) {
	}

	Element_Monitor_Pointer(Pointer pointer)
	    : Pre_Construct([this]() { before_construct(); })
	    , m_pointer(pointer)
	    , m_base_pointer(base_pointer(pointer)) {
		Handler::process_object_event(
		    Object_Event(Object_Event_Type::Construct, &m_base_pointer));
	}

	~Element_Monitor_Pointer() {
		Handler::process_object_event(
		    Object_Event(Object_Event_Type::Destroy, &m_base_pointer));
	}

	Element_Monitor_Pointer(Element_Monitor_Pointer const &element)
	    : Pre_Construct([this]() { before_construct(); })
	    , m_pointer(element.m_pointer)
	    , m_base_pointer(element.m_base_pointer) {
		Handler::process_object_event(Object_Event(
		    Object_Event_Type::Copy_Construct,
		    &m_base_pointer,
		    &element.m_base_pointer));
	}

	Element_Monitor_Pointer(
	    Element_Monitor_Pointer<false, Base, Handler> const &element)
	requires IsConst
	    : Pre_Construct([this]() { before_construct(); })
	    , m_pointer(element.m_pointer)
	    , m_base_pointer(element.m_base_pointer) {
		Handler::process_object_event(Object_Event(
		    Object_Event_Type::Copy_Construct,
		    &m_base_pointer,
		    &element.m_base_pointer));
	}

	Element_Monitor_Pointer &operator=(Element_Monitor_Pointer const &element) {
		assign(Object_Event_Type::Copy_Assign, element);
		return *this;
	}

	Element_Monitor_Pointer &operator=(
	    Element_Monitor_Pointer<false, Base, Handler> const &element)
	requires IsConst
	{
		assign(Object_Event_Type::Copy_Assign, element);
		return *this;
	}

	auto operator=(std::nullptr_t) noexcept -> Element_Monitor_Pointer & {
		operator=(Pointer(nullptr));
		return *this;
	}

	auto operator=(Pointer pointer) noexcept -> Element_Monitor_Pointer & {
		m_pointer      = pointer;
		m_base_pointer = base_pointer(pointer);
		Handler::process_object_event(Object_Event(
		    Object_Event_Type::Underlying_Copy_Assign,
		    &m_base_pointer));
		return *this;
	}

	Element_Monitor_Pointer(Element_Monitor_Pointer &&element) noexcept
	    : Pre_Construct([this]() { before_construct(); })
	    , m_pointer(element.m_pointer)
	    , m_base_pointer(element.m_base_pointer) {
		Handler::process_object_event(Object_Event(
		    Object_Event_Type::Move_Construct,
		    &m_base_pointer,
		    &element.m_base_pointer));
	}

	Element_Monitor_Pointer(
	    Element_Monitor_Pointer<false, Base, Handler> &&element) noexcept
	requires IsConst
	    : Pre_Construct([this]() { before_construct(); })
	    , m_pointer(element.m_pointer)
	    , m_base_pointer(element.m_base_pointer) {
		Handler::process_object_event(Object_Event(
		    Object_Event_Type::Move_Construct,
		    &m_base_pointer,
		    &element.m_base_pointer));
	}

	Element_Monitor_Pointer &operator=(Element_Monitor_Pointer &&element) noexcept {
		assign(Object_Event_Type::Move_Assign, element);
		return *this;
	}

	Element_Monitor_Pointer &operator=(
	    Element_Monitor_Pointer<false, Base, Handler> &&element) noexcept
	requires IsConst
	{
		assign(Object_Event_Type::Move_Assign, element);
		return *this;
	}

	auto operator++() -> Element_Monitor_Pointer {
		m_pointer++;
		assign(Object_Event_Type::Underlying_Copy_Assign, m_pointer);
		return Element_Monitor_Pointer(m_pointer);
	}

	auto operator++(std::integral auto) -> const Element_Monitor_Pointer {
		Element_Monitor_Pointer element(m_pointer);
		m_pointer++;
		assign(Object_Event_Type::Underlying_Copy_Assign, m_pointer);
		return element;
	}

	auto operator--() -> Element_Monitor_Pointer {
		m_pointer--;
		assign(Object_Event_Type::Underlying_Copy_Assign, m_pointer);
		return Element_Monitor_Pointer(m_pointer);
	}

	auto operator--(std::integral auto) -> const Element_Monitor_Pointer {
		Element_Monitor_Pointer element(m_pointer);
		m_pointer--;
		assign(Object_Event_Type::Underlying_Copy_Assign, m_pointer);
		return element;
	}

	friend auto operator+(
	    Element_Monitor_Pointer const &element,
	    std::integral auto             offset) -> Element_Monitor_Pointer {
		return Element_Monitor_Pointer(element.m_pointer + offset);
	}

	auto operator+=(std::integral auto offset) -> Element_Monitor_Pointer & {
		m_pointer += offset;
		assign(Object_Event_Type::Underlying_Copy_Assign, m_pointer);
		return *this;
	}

	template<bool ParameterConst>
	friend auto operator-(
	    Element_Monitor_Pointer const &element,
	    Element_Monitor_Pointer<ParameterConst, Base, Handler> const &offset)
	    -> difference_type {
		return element.m_pointer - offset.m_pointer;
	}

	friend auto operator-(
	    Element_Monitor_Pointer const &element,
	    std::integral auto             offset) -> Element_Monitor_Pointer {
		return Element_Monitor_Pointer(element.m_pointer - offset);
	}

	auto operator-=(std::integral auto offset) -> Element_Monitor_Pointer & {
		m_pointer -= offset;
		assign(Object_Event_Type::Underlying_Copy_Assign, m_pointer);
		return *this;
	}

	template<bool ParameterConst>
	auto operator==(
	    Element_Monitor_Pointer<ParameterConst, Base, Handler> const &element) const
	    -> bool {
		return this->operator==(element.m_pointer);
	}

	auto operator==(Pointer element) const -> bool {
		return m_pointer == element;
	}

	template<bool ParameterConst>
	auto operator!=(
	    Element_Monitor_Pointer<ParameterConst, Base, Handler> const &element) const
	    -> bool {
		return !this->operator==(element.m_pointer);
	}

	auto operator!=(Pointer element) const -> bool {
		return !this->operator==(element);
	}

	auto operator<(Pointer element) const -> bool {
		return m_pointer < element;
	}

	template<bool ParameterConst>
	auto operator<(
	    Element_Monitor_Pointer<ParameterConst, Base, Handler> const &element) const
	    -> bool {
		return this->operator<(element.m_pointer);
	}

	auto operator[](std::integral auto index) const -> Interface_Reference {
		return m_pointer[index];
	}

	auto operator*() const -> Interface_Reference {
		return *m_pointer;
	}

	auto operator->() const -> Interface_Pointer {
		return m_pointer;
	}

	auto get() const -> Interface_Pointer {
		return m_pointer;
	}

	auto base() -> Base_Pointer & {
		return m_base_pointer;
	}

	friend auto operator<<(
	    std::ostream                  &stream,
	    Element_Monitor_Pointer const &pointer) -> std::ostream & {
		return stream << pointer.get();
	}

 private:
	Pointer      m_pointer      = nullptr;
	Base_Pointer m_base_pointer = nullptr;

	void before_construct() {
		Handler::process_object_event(Object_Event(
		    Object_Event_Type::Before_Construct,
		    &m_base_pointer));
	}

	Base_Pointer base_pointer(Pointer pointer) {
		if (pointer == nullptr)
		{
			return nullptr;
		}

		return &(pointer->base());
	}

	void assign(Object_Event_Type type, Element_Monitor_Pointer const &element) {
		m_pointer      = element.m_pointer;
		m_base_pointer = element.m_base_pointer;
		Handler::process_object_event(
		    Object_Event(type, &m_base_pointer, &element.m_base_pointer));
	}

	void assign(Object_Event_Type type, Pointer const &pointer) {
		m_pointer      = pointer;
		m_base_pointer = base_pointer(pointer);
		Handler::process_object_event(Object_Event(type, &m_base_pointer));
	}
};

/// @brief Monitors the allocations and object lifetime within those allocations
///
/// @description Tracks certain actions and calls the callback provided in
/// Memory_Monitor_Event_Handler. This allows external code to peek at the state
/// of elements within a container. A consequence of the implementation is that
/// the lifetime of stack variables is tracked aswell. The event handler must
/// figure out which events they are interested in by keeping the necessary
/// state.
template<typename Value_t, Memory_Monitor_Event_Handler<Value_t> Handler>
class Memory_Monitor
{
 public:
	using Underlying_Value   = Value_t;
	using Underlying_Pointer = Value_t *;

 public:
	template<typename T>
	using rebind = Memory_Monitor<T, Handler>;

	// clang-format off
	using Value         = Element_Monitor<Underlying_Value, Handler>;
	using Pointer       = Element_Monitor_Pointer<false, Underlying_Value, Handler>;
	using Const_Pointer = Element_Monitor_Pointer<true, Underlying_Value, Handler>;
	// clang-format on

	explicit Memory_Monitor() = default;

	template<typename T>
	explicit Memory_Monitor(Memory_Monitor<T, Handler> /* monitor */)
	    : Memory_Monitor() {
	}

	~Memory_Monitor() = default;

	auto allocate(std::size_t count) -> Pointer {
		Allocator allocator;
		Pointer   address = Alloc_Traits::allocate(allocator, count);
		Handler::process_allocation_event(Allocation_Event(
		    Allocation_Event_Type::Allocate,
		    &address->base(),
		    count));
		return address;
	}

	template<typename... Arguments>
	constexpr void construct(Pointer address, Arguments &&...arguments) {
		Allocator allocator;
		Alloc_Traits::construct(
		    allocator,
		    address.get(),
		    std::forward<Arguments>(arguments)...);
	}

	constexpr void destroy(Pointer address) {
		Allocator allocator;
		Alloc_Traits::destroy(allocator, address.get());
	}

	void deallocate(Pointer address, std::size_t count) {
		Allocation_Event event(
		    Allocation_Event_Type::Deallocate,
		    &address->base(),
		    count);

		Allocator allocator;
		if (Handler::before_deallocate(event))
		{
			Alloc_Traits::deallocate(allocator, address.get(), count);
			Handler::process_allocation_event(event);
		}
	}

 private:
	using Allocator    = Default_Allocator<Value>;
	using Alloc_Traits = Allocator_Traits<Allocator>;
};

} // namespace dsa

#endif
