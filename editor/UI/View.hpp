/*
 * ---------------------------------------------------
 * View.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef VIEW_HPP
#define VIEW_HPP

#include <concepts>
#include <ranges>

namespace GE_Editor::UI
{

template<typename T>
concept View = requires(const T& view) {
    { view.render() } -> std::same_as<void>;
};

template<typename T>
concept ViewRange = std::ranges::range<T> && View<std::ranges::range_value_t<T>>;

}

#endif // VIEW_HPP
