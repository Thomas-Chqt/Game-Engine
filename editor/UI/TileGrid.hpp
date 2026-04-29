/*
 * ---------------------------------------------------
 * TileGrid.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef TILEGRID_HPP
#define TILEGRID_HPP

#include "UI/View.hpp"

#include <imgui.h>

namespace GE_Editor::UI
{

template<ViewRange ViewRange>
struct TileGrid
{
    ViewRange m_viewRange;

    void render() const
    {
        for (int i = 0; const auto& view : m_viewRange)
        {
            float posA = ImGui::GetCursorScreenPos().x;
            view.render();
            ImGui::SameLine();
            float posB = ImGui::GetCursorScreenPos().x;
            float tileSize = posB - posA;
            if (tileSize > ImGui::GetContentRegionAvail().x)
                ImGui::NewLine();
        }
    }
};

}

#endif
