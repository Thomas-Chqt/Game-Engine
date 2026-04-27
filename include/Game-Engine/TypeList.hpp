/*
 * ---------------------------------------------------
 * TypeList.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2026/04/26
 * ---------------------------------------------------
 */

#ifndef TYPELIST_HPP
#define TYPELIST_HPP

#include <concepts>
#include <type_traits>
#include <utility>

namespace GE
{

template<typename... Ts>
struct TypeList
{
    template<template<typename...> typename Container>
    using into = Container<Ts...>;

    template<template<typename> typename WrappedType>
    using wrapped = TypeList<WrappedType<Ts>...>;
};

template<typename T, typename TList>
struct TypeListContains;

template<typename T, typename... Ts>
struct TypeListContains<T, TypeList<Ts...>> : std::bool_constant<(std::is_same_v<T, Ts> || ...)>
{
};

template<typename T, typename TList>
concept IsTypeInList = TypeListContains<T, TList>::value;

template<typename TList, typename Fn>
inline constexpr void forEachType(Fn&& fn)
{
    auto helper = [&]<typename... Ts>(TypeList<Ts...>) requires (std::same_as<decltype(fn.template operator()<Ts>()), void> && ...) {
        (fn.template operator()<Ts>(), ...);
    };
    return helper(TList{});
}

template<typename TList, typename Fn>
inline constexpr bool anyOfType(Fn&& fn)
{
    auto helper = [&]<typename... Ts>(TypeList<Ts...>) requires (std::convertible_to<decltype(fn.template operator()<Ts>()), bool> && ...) {
        return (static_cast<bool>(fn.template operator()<Ts>()) || ...);
    };
    return helper(TList{});
}

template<typename TList, typename Fn>
inline constexpr bool allOfType(Fn&& fn)
{
    auto helper = [&]<typename... Ts>(TypeList<Ts...>) requires (std::convertible_to<decltype(fn.template operator()<Ts>()), bool> && ...) {
        return (static_cast<bool>(fn.template operator()<Ts>()) && ...);
    };
    return helper(TList{});
}

template<typename TList, typename Fn>
inline constexpr bool noneOfType(Fn&& fn)
{
    return !anyOfType<TList>(std::forward<Fn>(fn));
}

} // namespace GE

#endif // TYPELIST_HPP
