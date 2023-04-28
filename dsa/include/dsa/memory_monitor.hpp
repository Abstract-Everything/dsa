#ifndef DSA_MEMORY_MONITOR_HPP
#define DSA_MEMORY_MONITOR_HPP

#include <dsa/allocator_traits.hpp>
#include <dsa/default_allocator.hpp>

#include <cassert>
#include <cstddef>
#include <memory>

namespace dsa
{

namespace detail
{

/// @brief Allows the Element_Monitor to send events before construction starts
class Pre_Construct
{
 public:
	explicit Pre_Construct(std::invocable auto callback)
	{
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

	explicit Element_Monitor_Base(Base const &base) : Base(base)
	{
	}

	explicit Element_Monitor_Base(Base &&base) : Base(std::move(base))
	{
	}

	auto base() -> Base &
	{
		return *this;
	}

	auto base() const -> Base const &
	{
		return *this;
	}
};

template<typename Base>
class Element_Monitor_Base<Base, false>
{
 public:
	Element_Monitor_Base() = default;

	explicit Element_Monitor_Base(Base base) : m_base(base)
	{
	}

	operator Base const &() const
	{
		return m_base;
	}

	auto base() -> Base &
	{
		return m_base;
	}

	auto base() const -> Base const &
	{
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
    -> std::ostream &
{
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
    -> std::ostream &
{
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
	Allocation_Event(Allocation_Event_Type type, T *pointer, size_t count)
	    : m_type(type)
	    , m_pointer(pointer)
	    , m_count(count)
	{
	}

	auto operator==(Allocation_Event const &event) const -> bool = default;

	friend auto operator<<(std::ostream &stream, Allocation_Event const &event)
	    -> std::ostream &
	{
		stream << event.m_type << " at address " << event.m_pointer
		       << " with count " << event.m_count;
		return stream;
	}

	auto type() -> Allocation_Event_Type
	{
		return m_type;
	}

	auto address() -> T *
	{
		return m_pointer;
	}

	auto count() -> size_t
	{
		return m_count;
	}

 private:
	Allocation_Event_Type m_type;
	T		     *m_pointer;
	size_t                m_count;
};

/// Describes the parameters passed to functions which modify an object or its
/// lifetime.
template<typename T>
class Object_Event
{
 public:
	Object_Event(Object_Event_Type type, T *destination)
	    : Object_Event(type, destination, nullptr)
	{
	}

	Object_Event(Object_Event_Type type, T *destination, T const *source)
	    : m_type(type)
	    , m_destination(destination)
	    , m_source(source)
	{
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
	}

	auto operator==(Object_Event const &event) const -> bool = default;

	friend auto operator<<(std::ostream &stream, Object_Event const &event)
	    -> std::ostream &
	{
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

	auto type() -> Object_Event_Type
	{
		return m_type;
	}

	auto destination() -> T *
	{
		return m_destination;
	}

	auto source() -> T const *
	{
		return m_source;
	}

	[[nodiscard]] auto constructing() const -> bool
	{
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

	[[nodiscard]] auto copying() const -> bool
	{
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

	[[nodiscard]] auto moving() const -> bool
	{
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
	T		 *m_destination;
	T const          *m_source;
};

template<typename Handler, typename Type>
concept Memory_Monitor_Event_Handler = requires(
    Allocation_Event<Type> allocation_event,
    Object_Event<Type>     object_event)
{
	{Handler::before_deallocate(std::move(allocation_event))} -> std::same_as<bool>;

	{Handler::process_allocation_event(std::move(allocation_event))};
	{Handler::process_object_event(std::move(object_event))};
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

	/// @brief Wraps around a value in order to monitor its lifetime
	class Element_Monitor
	    : private detail::Pre_Construct
	    , public detail::Element_Monitor_Base<Value_t>
	{
		using Base         = Value_t;
		using Base_Wrapper = detail::Element_Monitor_Base<Base>;

	 public:
		template<typename... Arguments>
		Element_Monitor(Arguments &&...arguments)
		    requires std::is_constructible_v<Base, Arguments...>
		    : Pre_Construct([this]() { before_construct(); })
		    , Base_Wrapper(std::forward<Arguments>(arguments)...)
		{
			Handler::process_object_event(Object_Event(
			    Object_Event_Type::Construct,
			    &Base_Wrapper::base()));
		}

		~Element_Monitor()
		{
			Handler::process_object_event(Object_Event(
			    Object_Event_Type::Destroy,
			    &Base_Wrapper::base()));
		}

		Element_Monitor(Element_Monitor const &element)
		    requires std::is_constructible_v<Base>
		    : Pre_Construct([this]() { before_construct(); })
		    , Base_Wrapper(element)
		{
			Handler::process_object_event(Object_Event(
			    Object_Event_Type::Copy_Construct,
			    &Base_Wrapper::base(),
			    &element.base()));
		}

		auto operator=(Element_Monitor const &element)
		    -> Element_Monitor &
		    requires std::is_assignable_v<Base &, Base>
		{
			Base_Wrapper::base() = element.base();
			Handler::process_object_event(Object_Event(
			    Object_Event_Type::Copy_Assign,
			    &Base_Wrapper::base(),
			    &element.base()));
			return *this;
		}

		auto operator=(Base const &value) noexcept
		    -> Element_Monitor &
		    requires std::is_assignable_v<Base &, Base>
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
		    , Base_Wrapper(std::move(element))
		{
			Handler::process_object_event(Object_Event(
			    Object_Event_Type::Move_Construct,
			    &Base_Wrapper::base(),
			    &element.base()));
		}

		auto operator=(Element_Monitor &&element) noexcept
		    -> Element_Monitor &
		    requires std::is_move_assignable_v<Base>
		{
			Base_Wrapper::base() = std::move(element.base());
			Handler::process_object_event(Object_Event(
			    Object_Event_Type::Move_Assign,
			    &Base_Wrapper::base(),
			    &element.base()));
			return *this;
		}

		auto operator=(Base &&value) noexcept
		    -> Element_Monitor &
		    requires std::is_move_assignable_v<Base>
		{
			Base_Wrapper::base() = value;
			Handler::process_object_event(Object_Event(
			    Object_Event_Type::Underlying_Move_Assign,
			    &Base_Wrapper::base()));
			return *this;
		}

	 private:
		void before_construct()
		{
			Handler::process_object_event(Object_Event(
			    Object_Event_Type::Before_Construct,
			    &Base_Wrapper::base()));
		}
	};

	using Value   = Element_Monitor;
	using Pointer = Value *;

	explicit Memory_Monitor() = default;

	template<typename T>
	explicit Memory_Monitor(Memory_Monitor<T, Handler> /* monitor */)
	    : Memory_Monitor()
	{
	}

	~Memory_Monitor() = default;

	auto allocate(std::size_t count) -> Pointer
	{
		Allocator allocator;
		Pointer   address = Alloc_Traits::allocate(allocator, count);
		Handler::process_allocation_event(Allocation_Event(
		    Allocation_Event_Type::Allocate,
		    &address->base(),
		    count));
		return address;
	}

	void deallocate(Pointer address, std::size_t count)
	{
		Allocation_Event event(
		    Allocation_Event_Type::Deallocate,
		    &address->base(),
		    count);

		Allocator allocator;
		if (Handler::before_deallocate(event))
		{
			Alloc_Traits::deallocate(allocator, address, count);
			Handler::process_allocation_event(event);
		}
	}

 private:
	using Allocator    = Default_Allocator<Value>;
	using Alloc_Traits = Allocator_Traits<Allocator>;
};

} // namespace dsa

#endif
