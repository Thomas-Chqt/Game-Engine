/*
 * ---------------------------------------------------
 * Group.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef GROUP_HPP
#define GROUP_HPP

#include "UI/View.hpp"
#include "UI/ViewBase.hpp"

#include <imgui.h>

#include <cstddef>
#include <tuple>

namespace GE_Editor::UI
{

template<View ... Views>
class Group : public ViewBase<Group<Views...>>
{
public:
    Group(Views... views)
        : m_views({std::move(views)...})
    {
    }

    void render() const
    {
        ImGui::BeginGroup();
        std::apply([](auto&& ... views) { ((views.render()), ...); }, m_views);
        ImGui::EndGroup();
    }

private:
    std::tuple<Views...> m_views;
};

}

#endif // GROUP_HPP
