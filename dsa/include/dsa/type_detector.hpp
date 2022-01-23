#ifndef DSA_TYPE_DETECTOR_HPP
#define DSA_TYPE_DETECTOR_HPP

#include <type_traits>

namespace dsa
{

namespace detail
{

struct No_Type
{
};

template<typename Always_Void, template<typename...> typename Operator, typename... Arguments>
struct Detector
{
	using Type    = No_Type;
	using Value_t = std::false_type;
};

template<template<typename...> typename Operator, typename... Arguments>
struct Detector<std::void_t<Operator<Arguments...>>, Operator, Arguments...>
{
	using Type    = decltype(Operator<Arguments...>());
	using Value_t = std::true_type;
};

} // namespace detail

template<template<typename...> typename Operator, typename... Arguments>
using Detect_T = typename detail::Detector<void, Operator, Arguments...>::Type;

template<template<typename...> typename Operator, typename... Arguments>
constexpr bool Detect_V = detail::Detector<void, Operator, Arguments...>::Value_t::value;

} // namespace dsa

#endif
