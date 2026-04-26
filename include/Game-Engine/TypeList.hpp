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
    [&]<typename... Ts>(TypeList<Ts...>) {
        auto&& callback = fn;
        static_assert((std::is_void_v<decltype(callback.template operator()<Ts>())> && ...), "forEachType callback must return void.");
        (callback.template operator()<Ts>(), ...);
    }(TList{});
}

template<typename TList, typename Fn>
inline constexpr bool anyOfType(Fn&& fn)
{
    return [&]<typename... Ts>(TypeList<Ts...>) {
        auto&& callback = fn;
        static_assert((std::is_convertible_v<decltype(callback.template operator()<Ts>()), bool> && ...), "anyOfType callback must return bool-convertible values.");
        return (static_cast<bool>(callback.template operator()<Ts>()) || ...);
    }(TList{});
}

template<typename TList, typename Fn>
inline constexpr bool allOfType(Fn&& fn)
{
    return [&]<typename... Ts>(TypeList<Ts...>) {
        auto&& callback = fn;
        static_assert((std::is_convertible_v<decltype(callback.template operator()<Ts>()), bool> && ...), "allOfType callback must return bool-convertible values.");
        return (static_cast<bool>(callback.template operator()<Ts>()) && ...);
    }(TList{});
}

template<typename TList, typename Fn>
inline constexpr bool noneOfType(Fn&& fn)
{
    return !anyOfType<TList>(std::forward<Fn>(fn));
}

} // namespace GE

#endif // TYPELIST_HPP
