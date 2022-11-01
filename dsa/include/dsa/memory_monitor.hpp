#ifndef DSA_MEMORY_MONITOR_HPP
#define DSA_MEMORY_MONITOR_HPP

#include <dsa/allocator_traits.hpp>
#include <dsa/default_allocator.hpp>

#include <cassert>
#include <cstddef>
#include <memory>

namespace dsa
{

template<typename Handler, typename Type>
concept Memory_Monitor_Event_Handler =
    requires(Handler handler, Type *destination, Type const *source, size_t count)
{
	{handler.on_allocate(destination, count)};

	{handler.on_construct(destination)};

	{handler.on_copy_construct(destination, source)};
	{handler.on_copy_assign(destination, source)};
	{handler.on_underlying_value_copy_assign(destination)};

	{handler.on_move_construct(destination, source)};
	{handler.on_move_assign(destination, source)};
	{handler.on_underlying_value_move_assign(destination)};

	{handler.on_destroy(destination)};

	{handler.before_deallocate(destination, count)} -> std::same_as<bool>;
	{handler.on_deallocate(destination, count)};
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
	/// @brief Wraps around a value in order to monitor its lifetime
	class Element_Monitor
	{
	 public:
		template<typename... Arguments>
		explicit Element_Monitor(Arguments &&...arguments)
		    : m_value(std::forward<Arguments>(arguments)...)
		{
			handler()->on_construct(this);
		}

		~Element_Monitor()
		{
			handler()->on_destroy(this);
		}

		Element_Monitor(Element_Monitor const &element)
		    : m_value(element.m_value)
		{
			handler()->on_copy_construct(this, &element);
		}

		auto operator=(Element_Monitor const &element) -> Element_Monitor &
		{
			m_value = element.m_value;
			handler()->on_copy_assign(this, &element);
			return *this;
		}

		auto operator=(Value_t const &value) -> Element_Monitor &
		{
			m_value = value;
			handler()->on_underlying_value_copy_assign(this);
			return *this;
		}

		Element_Monitor(Element_Monitor &&element) noexcept
		    : m_value(std::move(element.m_value))
		{
			handler()->on_move_construct(this, &element);
		}

		auto operator=(Element_Monitor &&element) noexcept
		    -> Element_Monitor &
		{
			m_value = std::move(element.m_value);
			handler()->on_move_assign(this, &element);
			return *this;
		}

		auto operator=(Value_t &&value) noexcept -> Element_Monitor &
		{
			m_value = std::move(value);
			handler()->on_underlying_value_move_assign(this);
			return *this;
		}

		explicit operator Value_t &()
		{
			return m_value;
		}

		explicit operator Value_t const &() const
		{
			return m_value;
		}

	 private:
		Value_t m_value{};
	};

	using Underlying_Value = Value_t;
	using Value            = Element_Monitor;
	using Pointer          = Value *;

	explicit Memory_Monitor()
	{
		assert(
	    handler() != nullptr
	    && "Only one Monitor instance can be alive at any given time");
	}

	~Memory_Monitor() = default;

	static auto handler() -> std::unique_ptr<Handler> &
	{
		static std::unique_ptr<Handler> handler;
		return handler;
	}

	auto allocate(std::size_t count) -> Pointer
	{
		Allocator allocator;
		Pointer   address = Alloc_Traits::allocate(allocator, count);
		handler()->on_allocate(address, count);
		return address;
	}

	void deallocate(Pointer address, std::size_t count)
	{
		Allocator allocator;
		if (handler()->before_deallocate(address, count))
		{
			Alloc_Traits::deallocate(allocator, address, count);
			handler()->on_deallocate(address, count);
		}
	}

 private:
	using Allocator    = Default_Allocator<Value>;
	using Alloc_Traits = Allocator_Traits<Allocator>;
};

} // namespace dsa

#endif
