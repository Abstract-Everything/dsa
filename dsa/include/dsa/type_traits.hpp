#ifndef DSA_TYPE_TRAITS_HPP
#define DSA_TYPE_TRAITS_HPP

#include <type_traits>

namespace dsa
{

template<typename T>
constexpr bool Has_Const = !std::is_same_v<T, std::remove_const_t<T>>;

template<typename Non_Const_T, typename Const_T>
constexpr bool Is_Const_Version_Of =
    Has_Const<Const_T> && std::is_same_v<Non_Const_T, std::remove_const_t<Const_T>>;

template<typename Type, typename... Arguments>
struct Is_Same : std::false_type
{
};

template<typename Type, typename T>
struct Is_Same<Type, T>
    : std::conditional_t<
	  std::is_same_v<std::decay_t<Type>, std::decay_t<T>>,
	  std::true_type,
	  std::false_type>
{
};

template<typename Type, typename... Arguments>
constexpr bool Is_Same_v = Is_Same<Type, Arguments...>::value;

template<typename... Arguments>
struct Overloaded_Lambda : Arguments...
{
 public:
	using Arguments::operator()...;
};

template<typename... Arguments>
Overloaded_Lambda(Arguments...) -> Overloaded_Lambda<Arguments...>;

} // namespace dsa

#endif
