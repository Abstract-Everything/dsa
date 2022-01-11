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

} // namespace dsa

#endif
