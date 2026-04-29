/*
 * ---------------------------------------------------
 * Pane.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef PANE_HPP
#define PANE_HPP

#include "UI/View.hpp"

#include <imgui.h>

#include <string_view>
#include <tuple>

namespace GE_Editor::UI
{

template<View ... Views>
class Pane
{
public:
    Pane(std::string_view name, Views... views)
        : m_name(std::move(name))
        , m_views({std::move(views)...})
    {
    }

    void render() const
    {
        ImGui::Begin(m_name.data());
        std::apply([](auto&& ... views) { ((views.render()), ...); }, m_views);
        ImGui::End();
    }

    std::string_view m_name;
    std::tuple<Views...> m_views;
};

}

#endif
