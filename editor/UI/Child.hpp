/*
 * ---------------------------------------------------
 * Child.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef CHILD_HPP
#define CHILD_HPP

#include "UI/View.hpp"

#include <imgui.h>

#include <string_view>
#include <tuple>

namespace GE_Editor::UI
{

template<View ... Views>
class Child
{
public:
    Child(std::string_view id, Views... views)
        : m_id(std::move(id))
        , m_views({std::move(views)...})
    {
    }

    void render() const
    {
        ImGui::BeginChild(m_id.data());
        std::apply([](auto&& ... views) { ((views.render()), ...); }, m_views);
        ImGui::EndChild();
    }

    std::string_view m_id;
    std::tuple<Views...> m_views;
};

}

#endif
