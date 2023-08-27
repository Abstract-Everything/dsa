#ifndef DSA_TYPE_DETECTOR_HPP
#define DSA_TYPE_DETECTOR_HPP

#include <type_traits>

namespace dsa
{

namespace detail
{

struct No_Type
{};

template<typename Default_Value, typename Always_Void, template<typename...> typename Operator, typename... Arguments>
struct Detector
{
	using Type    = Default_Value;
	using Value_t = std::false_type;
};

template<typename Default_Value, template<typename...> typename Operator, typename... Arguments>
struct Detector<Default_Value, std::void_t<Operator<Arguments...>>, Operator, Arguments...>
{
	using Type    = Operator<Arguments...>;
	using Value_t = std::true_type;
};

} // namespace detail

template<template<typename...> typename Operator, typename... Arguments>
using Detect_T = typename detail::Detector<detail::No_Type, void, Operator, Arguments...>::Type;

template<template<typename...> typename Operator, typename... Arguments>
constexpr bool Detect_V =
    detail::Detector<detail::No_Type, void, Operator, Arguments...>::Value_t::value;

template<typename Default_Value, template<typename...> typename Operator, typename... Arguments>
using Detect_Default = detail::Detector<Default_Value, void, Operator, Arguments...>;

template<typename Default_Value, template<typename...> typename Operator, typename... Arguments>
using Detect_Default_T = typename Detect_Default<Default_Value, Operator, Arguments...>::Type;

template<typename Default_Value, template<typename...> typename Operator, typename... Arguments>
constexpr bool Detect_Default_V =
    Detect_Default<Default_Value, Operator, Arguments...>::Value_t::value;

} // namespace dsa

#endif
