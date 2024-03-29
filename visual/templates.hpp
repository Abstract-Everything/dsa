#ifndef VISUAL_TEMPLATES_HPP
#define VISUAL_TEMPLATES_HPP

#include <type_traits>
#include <variant>

namespace tmpl
{

namespace detail
{

template<class Always_Void, template<class...> class Op, class... Args>
struct detector
{
	using value_t = std::false_type;
};

template<template<class...> class Op, class... Args>
struct detector<std::void_t<Op<Args...>>, Op, Args...>
{
	using value_t = std::true_type;
};

template<template<class...> class Op, class... Args>
using is_detected = typename detail::detector<void, Op, Args...>::value_t;

template<template<class...> class Op, class... Args>
constexpr bool is_detected_v = is_detected<Op, Args...>::value;

} // namespace detail

#define DEFINE_HAS_OPERATION(name, member)                                              \
                                                                                        \
	template<typename T, typename... Params>                                        \
	using name##_t = decltype(std::declval<T>().member(std::declval<Params>()...)); \
                                                                                        \
	template<typename T, typename... Params>                                        \
	constexpr bool has_##name##_v =                                                 \
	    tmpl::detail::is_detected_v<name##_t, T, Params...> // Intentionally
								// ommitted ';'

#define DEFINE_HAS_MEMBER(name) DEFINE_HAS_OPERATION(member_##name, name)

#define DEFINE_HAS_OPERATOR_ACCESS() DEFINE_HAS_OPERATION(operator_access, operator[])

#define DEFINE_HAS_OPERATOR_DEREFERENCE() DEFINE_HAS_OPERATION(operator_dereference, operator*)

template<typename Variant_Type, std::size_t variant_index = 0>
constexpr void construct_variant_by_index(Variant_Type &variant, std::size_t index) {
	if constexpr (variant_index == std::variant_size_v<Variant_Type>)
	{
		return;
	}
	else
	{
		if (index == variant_index)
		{
			variant = std::variant_alternative_t<variant_index, Variant_Type>{};
		}
		else
		{
			construct_variant_by_index<Variant_Type, variant_index + 1>(variant, index);
		}
	}
}

} // namespace tmpl
#endif
