#ifndef VISUAL_TEMPLATES_HPP
#define VISUAL_TEMPLATES_HPP

#include <type_traits>
#include <variant>

#define DEFINE_HAS_MEMBER(member)                                        \
                                                                         \
	template<typename, typename = std::void_t<>>                     \
	struct has_member_##member : std::false_type                     \
	{                                                                \
	};                                                               \
                                                                         \
	template<typename T>                                             \
	struct has_member_##member<T, std::void_t<decltype(&T::member)>> \
	    : std::true_type                                             \
	{                                                                \
	};                                                               \
                                                                         \
	template<typename T>                                             \
	constexpr bool has_member_##member##_v =                         \
	    has_member_##member<T>::value // Intentionally ommitted ';'

#define DEFINE_HAS_MEMBER_OVERLOADED(member)                                             \
                                                                                         \
	template<typename, typename = std::void_t<>>                                     \
	struct has_member_##member : std::false_type                                     \
	{                                                                                \
	};                                                                               \
                                                                                         \
	template<typename T>                                                             \
	struct has_member_##member<T, std::void_t<decltype(std::declval<T>().member())>> \
	    : std::true_type                                                             \
	{                                                                                \
	};                                                                               \
                                                                                         \
	template<typename T>                                                             \
	constexpr bool has_member_##member##_v =                                         \
	    has_member_##member<T>::value // Intentionally ommitted ';'

template<typename, typename = std::void_t<>>
struct has_member_operator_access : std::false_type
{
};

template<typename T>
struct has_member_operator_access<T, std::void_t<decltype(std::declval<T>()[0])>>
    : std::true_type
{
};

template<typename T>
constexpr bool has_member_operator_access_v =
    has_member_operator_access<T>::value;

template<typename Variant_Type, std::size_t variant_index = 0>
constexpr void construct_variant_by_index(Variant_Type &variant, std::size_t index)
{
	if constexpr (variant_index == std::variant_size_v<Variant_Type>)
	{
		return;
	}
	else
	{
		if (index == variant_index)
		{
			variant =
			    std::variant_alternative_t<variant_index, Variant_Type>{};
		}
		else
		{
			construct_variant_by_index<Variant_Type, variant_index + 1>(
			    variant,
			    index);
		}
	}
}

#endif
