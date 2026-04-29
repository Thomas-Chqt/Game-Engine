/*
 * ---------------------------------------------------
 * SameLine.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef SAMELINE_HPP
#define SAMELINE_HPP

#include "UI/View.hpp"

#include <imgui.h>

namespace GE_Editor::UI
{

template<View View1, View View2>
class SameLine
{
public:
    SameLine(View1 view1, View2 view2)
        : m_view1(std::move(view1))
        , m_view2(std::move(view2))
    {
    }

    void render() const
    {
        m_view1.render();
        ImGui::SameLine();
        m_view2.render();
    }

private:
    View1 m_view1;
    View2 m_view2;
};

}

#endif // SameLine_HPP
