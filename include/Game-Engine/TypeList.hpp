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
inline constexpr void forEachType(Fn&& fn);

template<typename... Ts, typename Fn>
inline constexpr void forEachType(TypeList<Ts...>, Fn&& fn)
{
    auto&& callback = fn;
    (callback.template operator()<Ts>(), ...);
}

template<typename TList, typename Fn>
inline constexpr void forEachType(Fn&& fn)
{
    forEachType(TList{}, std::forward<Fn>(fn));
}

} // namespace GE

#endif // TYPELIST_HPP
