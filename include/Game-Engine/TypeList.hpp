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

template<typename TList, template<typename> typename WrappedType>
struct TypeListWrap;

template<typename... Ts, template<typename> typename WrappedType>
struct TypeListWrap<TypeList<Ts...>, WrappedType>
{
    using type = TypeList<WrappedType<Ts>...>;
};

template<typename TList, template<typename> typename WrappedType>
using TypeListWrap_t = typename TypeListWrap<TList, WrappedType>::type;

template<typename TList, typename Fn>
inline constexpr void forEachType(Fn&& fn);

template<typename... Ts, typename Fn>
inline constexpr void forEachType(TypeList<Ts...>, Fn&& fn)
{
    auto&& callback = fn;
    static_assert((std::is_void_v<decltype(callback.template operator()<Ts>())> && ...), "forEachType callback must return void.");
    (callback.template operator()<Ts>(), ...);
}

template<typename TList, typename Fn>
inline constexpr void forEachType(Fn&& fn)
{
    forEachType(TList{}, std::forward<Fn>(fn));
}

template<typename TList, typename Fn>
inline constexpr bool anyOfType(Fn&& fn);

template<typename... Ts, typename Fn>
inline constexpr bool anyOfType(TypeList<Ts...>, Fn&& fn)
{
    auto&& callback = fn;
    static_assert((std::is_convertible_v<decltype(callback.template operator()<Ts>()), bool> && ...), "anyOfType callback must return bool-convertible values.");
    return (static_cast<bool>(callback.template operator()<Ts>()) || ...);
}

template<typename TList, typename Fn>
inline constexpr bool anyOfType(Fn&& fn)
{
    return anyOfType(TList{}, std::forward<Fn>(fn));
}

template<typename TList, typename Fn>
inline constexpr bool allOfType(Fn&& fn);

template<typename... Ts, typename Fn>
inline constexpr bool allOfType(TypeList<Ts...>, Fn&& fn)
{
    auto&& callback = fn;
    static_assert((std::is_convertible_v<decltype(callback.template operator()<Ts>()), bool> && ...), "allOfType callback must return bool-convertible values.");
    return (static_cast<bool>(callback.template operator()<Ts>()) && ...);
}

template<typename TList, typename Fn>
inline constexpr bool allOfType(Fn&& fn)
{
    return allOfType(TList{}, std::forward<Fn>(fn));
}

template<typename TList, typename Fn>
inline constexpr bool noneOfType(Fn&& fn);

template<typename... Ts, typename Fn>
inline constexpr bool noneOfType(TypeList<Ts...>, Fn&& fn)
{
    return !anyOfType(TypeList<Ts...>{}, std::forward<Fn>(fn));
}

template<typename TList, typename Fn>
inline constexpr bool noneOfType(Fn&& fn)
{
    return noneOfType(TList{}, std::forward<Fn>(fn));
}

} // namespace GE

#endif // TYPELIST_HPP
